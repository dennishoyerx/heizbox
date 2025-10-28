// src/Device.cpp (Vereinfacht)
#include "Device.h"
#include "nvs_flash.h"
#include "credentials.h"
#include "config.h"
#include <ArduinoOTA.h>
#include "MenuBuilder.h"
#include "ScreenRegistry.h"
#include "ScreenBase.h"
#include "StateManager.h"
#include "Logger.h"
#include <utility> // For std::move and std::make_unique

// ============================================================================
// State Bindings - UI ↔ State Synchronisation
// ============================================================================

class StateBinding {
public:
    // Display Brightness binden
    static void bindBrightness(DisplayManager* display) {
        auto& state = STATE;

        // Initial sync
        display->setBrightness(state.brightness.get());

        // Listen to state changes
        state.brightness.addListener([display](uint8_t value) {
            display->setBrightness(value);
        });
    }

    // Dark Mode binden
    static void bindDarkMode(DisplayManager* display) {
        auto& state = STATE;

        // Initial sync
        display->setDarkMode(state.darkMode.get());

        // Listen to state changes
        state.darkMode.addListener([display](bool enabled) {
            display->setDarkMode(enabled);
        });
    }

    // Timezone binden
    static void bindTimezone(ClockManager* clock) {
        auto& state = STATE;

        clock->setTimezoneOffset(state.timezoneOffset.get());

        state.timezoneOffset.addListener([clock](int32_t offset) {
            clock->setTimezoneOffset(offset);
        });
    }

    // Auto-Stop Time binden
    static void bindAutoStopTime(HeaterController* heater) {
        auto& state = STATE;

        heater->setAutoStopTime(state.autoStopTime.get());

        state.autoStopTime.addListener([heater](uint32_t time) {
            heater->setAutoStopTime(time);
        });
    }

    // Alle Bindings auf einmal
    static void bindAll(DisplayManager* display, ClockManager* clock,
                       HeaterController* heater) {
        bindBrightness(display);
        bindDarkMode(display);
        bindTimezone(clock);
        bindAutoStopTime(heater);
    }
};

Device::Device()
    : input(),
      heater(),
      display(&clockManager),
      clockManager(),
      statsManager(),
      wifiManager(),
      webSocketManager(),
      screenManager(display, input),
      fireScreen(heater, &screenManager, &screensaverScreen, &statsManager,
                 [this](int cycle) { this->setCurrentCycle(cycle); }),
      hiddenModeScreen(&display),
      screensaverScreen(clockManager, 30000, &display, [this]() {
          fireScreen.resetActivityTimer();
          NAVIGATE_TO(&screenManager, ScreenType::FIRE);
      }),
      otaUpdateScreen(&display),
      statsScreen(statsManager),
      timezoneScreen(clockManager, &screenManager),
      startupScreen([this]() {
          NAVIGATE_TO_WITH_TRANSITION(&screenManager, ScreenType::FIRE, ScreenTransition::FADE);
      }),
      lastSetCycle(1),
      mainMenuScreen(nullptr) // Wird später initialisiert
{}

Device::~Device() = default; // Destructor definition

void Device::setup() {
    Serial.begin(115200);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Initialize core components
    input.init();
    heater.init();
    statsManager.init();
    display.init(&screenManager);


    // Screen-Registry aufsetzen
    setupScreenRegistry();


        // Setup screens
        screenManager.setScreen(&startupScreen);

        input.setCallback([this](InputEvent event) {
            this->handleInput(event);
        });

    clockManager.init();

    // State-Bindings erstellen
    StateBinding::bindAll(&display, &clockManager, &heater);

    // Setup WiFi
    setupMainMenu();
    wifiManager.init(WIFI_SSID, WIFI_PASSWORD, "Heizbox");
    wifiManager.onConnectionChange([this](bool connected) {
        if (connected) {
            webSocketManager.init(Config::BACKEND_WS_URL, Config::DEVICE_ID, "device");
        }
    });

    // Setup WebSocket
    webSocketManager.onMessage([this](const char* type, const JsonDocument& doc) {
        this->handleWebSocketMessage(type, doc);
    });

    webSocketManager.onConnectionChange([](bool connected) {
        Serial.printf("🔌 WebSocket %s\n", connected ? "connected" : "disconnected");
    });

    // Setup OTA
    setupOTA();

    logPrint("test");

    Serial.println("✅ Device initialized");
}

void Device::setupScreenRegistry() {
    // Alle Screens registrieren
    REGISTER_SCREEN(ScreenType::STARTUP, startupScreen);
    REGISTER_SCREEN(ScreenType::FIRE, fireScreen);
    REGISTER_SCREEN(ScreenType::STATS, statsScreen);
    REGISTER_SCREEN(ScreenType::TIMEZONE, timezoneScreen);
    REGISTER_SCREEN(ScreenType::SCREENSAVER, screensaverScreen);
    REGISTER_SCREEN(ScreenType::OTA_UPDATE, otaUpdateScreen);
    REGISTER_SCREEN(ScreenType::HIDDEN_MODE, hiddenModeScreen);

    // Main menu wird später registriert (nach Erstellung)
}

void Device::setupMainMenu() {
    auto& state = STATE;

    auto menuItems = MenuBuilder()
        // Observable-Integration: Kein Cast, kein Callback nötig!
        .addObservableRange("Brightness", state.brightness,
                           static_cast<uint8_t>(20),
                           static_cast<uint8_t>(100),
                           static_cast<uint8_t>(10), "%")

        .addObservableToggle("Dark Mode", state.darkMode)

        .addAction("Timezone", [this]() {
            NAVIGATE_TO_WITH_TRANSITION(&screenManager, ScreenType::TIMEZONE, ScreenTransition::FADE);
        })

        .addAction("Stats", [this]() {
            NAVIGATE_TO_WITH_TRANSITION(&screenManager, ScreenType::STATS, ScreenTransition::FADE);
        })

        // Sleep Timeout: Intern Millisekunden, angezeigt als Sekunden
        .addObservableRangeMs("Sleep Timeout", state.sleepTimeout,
                             60000,    // 1 Minute min
                             1800000,  // 30 Minuten max
                             60000)    // 1 Minute step

        .addAction("Reset Session", [this]() {
            STATE.sessionCycles.set(0);
            STATE.sessionClicks.set(0);
            STATE.sessionCaps.set(0);
        })

        .addAction("FACTORY RESET", [this]() {
            nvs_flash_erase();
            esp_restart();
        })

        .build();

    mainMenuScreen = std::make_unique<GenericMenuScreen>("SETTINGS", std::move(menuItems));
    REGISTER_SCREEN(ScreenType::MAIN_MENU, *mainMenuScreen);

    // Setup timezone exit callback
    timezoneScreen.setCallback([this]() {
        NAVIGATE_TO_WITH_TRANSITION(&screenManager, ScreenType::MAIN_MENU, ScreenTransition::FADE);
    });
}

void Device::loop() {
    // Update managers
    wifiManager.update();
    webSocketManager.update();
    input.update();
    heater.update();
    clockManager.update();
    statsManager.update();

    // Update UI
    screenManager.update();
    screenManager.draw();
    display.renderStatusBar();

    // Check heating status and send updates
    checkHeatingStatus();

    // Check for completed heat cycles
    checkHeatCycle();

    // OTA handling
    ArduinoOTA.handle();

    delay(5);
}

// ============================================================================
// Helper Methods
// ============================================================================

void Device::setupOTA() {
    ArduinoOTA.setHostname("Heizbox");

    ArduinoOTA.onStart([this]() {
        logPrint("📲 OTA Update started");
        screenManager.setScreen(&otaUpdateScreen);
    });

    ArduinoOTA.onEnd([this]() {
        logPrint("📲 OTA Update completed");
        screenManager.setScreen(&fireScreen);
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        Serial.printf("📲 OTA Error[%u]: ", error);
        switch (error) {
            case OTA_AUTH_ERROR:    Serial.println("Auth Failed"); break;
            case OTA_BEGIN_ERROR:   Serial.println("Begin Failed"); break;
            case OTA_CONNECT_ERROR: Serial.println("Connect Failed"); break;
            case OTA_RECEIVE_ERROR: Serial.println("Receive Failed"); break;
            case OTA_END_ERROR:     Serial.println("End Failed"); break;
        }
        screenManager.setScreen(&fireScreen);
    });

    ArduinoOTA.begin();
    Serial.println("📲 OTA ready");
}

void Device::checkHeatingStatus() {
    const bool currentHeatingStatus = heater.isHeating();

    if (currentHeatingStatus != lastHeatingStatusSent) {
        STATE.isHeating.set(currentHeatingStatus);
        webSocketManager.sendStatusUpdate(true, currentHeatingStatus);
        lastHeatingStatusSent = currentHeatingStatus;
    }
}

void Device::checkHeatCycle() {
    if (heater.isCycleFinished()) {
        const uint32_t durationMs = heater.getLastCycleDuration();
        const uint32_t durationSec = durationMs / 1000;

        // Add to stats
        statsManager.addCycle(durationMs);

        // Update state
        STATE.totalCycles.update([](uint32_t val) { return val + 1; });
        STATE.sessionCycles.update([](uint32_t val) { return val + 1; });
        STATE.totalDuration.update([durationMs](uint32_t val) { return val + durationMs; });

        heater.clearCycleFinishedFlag();

        // Send to backend
        webSocketManager.sendHeatCycleCompleted(durationSec, lastSetCycle);

        Serial.printf("✅ Heat cycle completed: %lu seconds (cycle %d)\n",
                     durationSec, lastSetCycle);
    }
}

void Device::handleWebSocketMessage(const char* type, const JsonDocument& doc) {
    if (strcmp(type, "sessionData") == 0 || strcmp(type, "sessionUpdate") == 0) {
        if (!doc["clicks"].isNull()) {
            STATE.sessionClicks.set(doc["clicks"].as<int>());
        }
        if (!doc["caps"].isNull()) {
            STATE.sessionCaps.set(doc["caps"].as<int>());
        }
        if (!doc["consumption"].isNull()) {
            STATE.sessionConsumption.set(doc["consumption"].as<float>());
        }
        if (!doc["consumptionTotal"].isNull()) {
            STATE.todayConsumption.set(doc["consumptionTotal"].as<float>());
        }
        screenManager.setDirty();
    }
}

void Device::handleInput(InputEvent event) {
    const char* typeStr = (event.type == PRESS) ? "PRESS" :
                         (event.type == RELEASE) ? "RELEASE" : "HOLD";
    const char* btnStr = (event.button == UP) ? "UP" :
                        (event.button == DOWN) ? "DOWN" :
                        (event.button == LEFT) ? "LEFT" :
                        (event.button == RIGHT) ? "RIGHT" :
                        (event.button == CENTER) ? "CENTER" : "FIRE";

    Serial.printf("🎮 %s %s\n", btnStr, typeStr);

    // Handle global shortcuts
    if (handleGlobalShortcuts(event)) {
        return;
    }

    // Pass to screen manager
    screenManager.handleInput(event);
}

bool Device::handleGlobalShortcuts(InputEvent event) {
    const ScreenType currentScreen = screenManager.getCurrentScreenType();

    // HOLD LEFT: Toggle between Fire <-> MainMenu
    if (event.button == LEFT && event.type == HOLD) {
        ScreenType targetType = (currentScreen == ScreenType::FIRE)
            ? ScreenType::MAIN_MENU : ScreenType::FIRE;
        NAVIGATE_TO_WITH_TRANSITION(&screenManager, targetType, ScreenTransition::FADE);
        return true;
    }

    return false;
}

void Device::setCurrentCycle(int cycle) {
    lastSetCycle = cycle;
    Serial.printf("📊 Current cycle set to: %d\n", lastSetCycle);
}
