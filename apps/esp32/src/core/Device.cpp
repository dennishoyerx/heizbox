// src/Device.cpp (Vereinfacht)
#include "core/Device.h"
#include "nvs_flash.h"
#include "credentials.h"
#include "core/Config.h"
#include <ArduinoOTA.h>
#include "ui/components/MenuBuilder.h"
#include "core/StateManager.h"
#include "utils/Logger.h"
#include <utility> // For std::move and std::make_unique

// Hardware
#include "hardware/HeaterController.h"
#include "hardware/InputManager.h"
#include "hardware/DisplayDriver.h"
#include "hardware/ClockManager.h"

// Network
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"

// UI
#include "ui/base/ScreenManager.h"
#include "ui/screens/FireScreen.h"
#include "ui/screens/HiddenModeScreen.h"
#include "ui/screens/ScreensaverScreen.h"
#include "ui/screens/OtaUpdateScreen.h"
#include "ui/screens/StatsScreen.h"
#include "ui/screens/TimezoneScreen.h"
#include "ui/screens/StartupScreen.h"
#include "ui/base/ScreenTransition.h"




// ============================================================================
// State Bindings - UI ↔ State Synchronisation
// ============================================================================

class StateBinding {
public:
    // Display Brightness binden
    static void bindBrightness(DisplayDriver* display) {
        auto& state = DeviceState::instance();

        // Initial sync
        display->setBrightness(state.brightness.get());

        // Listen to state changes
        state.brightness.addListener([display](uint8_t value) {
            display->setBrightness(value);
        });
    }

    // Dark Mode binden
    static void bindDarkMode(DisplayDriver* display) {
        auto& state = DeviceState::instance();

        // Initial sync
        display->setDarkMode(state.darkMode.get());

        // Listen to state changes
        state.darkMode.addListener([display](bool enabled) {
            display->setDarkMode(enabled);
        });
    }

    // Timezone binden
    static void bindTimezone(ClockManager* clock) {
        auto& state = DeviceState::instance();

        clock->setTimezoneOffset(state.timezoneOffset.get());

        state.timezoneOffset.addListener([clock](int32_t offset) {
            clock->setTimezoneOffset(offset);
        });
    }

    // Auto-Stop Time binden
    static void bindAutoStopTime(HeaterController* heater) {
        auto& state = DeviceState::instance();

        heater->setAutoStopTime(state.autoStopTime.get());

        state.autoStopTime.addListener([heater](uint32_t time) {
            heater->setAutoStopTime(time);
        });
    }

    // Alle Bindings auf einmal
    static void bindAll(DisplayDriver* display, ClockManager* clock,
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
      screensaverScreen(clockManager, DeviceState::instance().sleepTimeout.get(), &display, [this]() {
          fireScreen.resetActivityTimer();
          screenManager.setScreen(&fireScreen);
      }),
      otaUpdateScreen(&display),
      statsScreen(statsManager),
      timezoneScreen(clockManager, &screenManager),
      startupScreen([this]() {
          screenManager.setScreen(&fireScreen, ScreenTransition::FADE);
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
            webSocketManager.init(NetworkConfig::BACKEND_WS_URL, NetworkConfig::DEVICE_ID, "device");
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
    screenManager.registerScreen(ScreenType::STARTUP, &startupScreen);
    screenManager.registerScreen(ScreenType::FIRE, &fireScreen);
    screenManager.registerScreen(ScreenType::STATS, &statsScreen);
    screenManager.registerScreen(ScreenType::TIMEZONE, &timezoneScreen);
    screenManager.registerScreen(ScreenType::SCREENSAVER, &screensaverScreen);
    screenManager.registerScreen(ScreenType::OTA_UPDATE, &otaUpdateScreen);
    screenManager.registerScreen(ScreenType::HIDDEN_MODE, &hiddenModeScreen);

    // Main menu wird später registriert (nach Erstellung)
}

void Device::setupMainMenu() {
    auto& state = DeviceState::instance();

    auto menuItems = MenuBuilder()
        // Observable-Integration: Kein Cast, kein Callback nötig!
        .addObservableRange("Brightness", state.brightness,
                           static_cast<uint8_t>(20),
                           static_cast<uint8_t>(100),
                           static_cast<uint8_t>(10), "%")

        .addObservableToggle("Dark Mode", state.darkMode)

        .addObservableToggle("Center Heat", state.enableCenterButtonForHeating)

        .addAction("Timezone", [this]() {
            screenManager.setScreen(&timezoneScreen, ScreenTransition::FADE);
        })

        .addAction("Stats", [this]() {
            screenManager.setScreen(&statsScreen, ScreenTransition::FADE);
        })

        // Sleep Timeout: Intern Millisekunden, angezeigt als Sekunden
        .addObservableRangeMs("Sleep Timeout", state.sleepTimeout,
                             60000,    // 1 Minute min
                             1800000,  // 30 Minuten max
                             60000)    // 1 Minute step

        .addAction("Reset Session", [this]() {
            DeviceState::instance().sessionCycles.set(0);
            DeviceState::instance().sessionClicks.set(0);
            DeviceState::instance().sessionCaps.set(0);
        })

        .addAction("FACTORY RESET", [this]() {
            nvs_flash_erase();
            esp_restart();
        })

        .build();

    mainMenuScreen = std::make_unique<GenericMenuScreen>("SETTINGS", std::move(menuItems));
    screenManager.registerScreen(ScreenType::MAIN_MENU, mainMenuScreen.get());

    // Setup timezone exit callback
    timezoneScreen.setCallback([this]() {
        screenManager.setScreen(mainMenuScreen.get(), ScreenTransition::FADE);
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
        DeviceState::instance().isHeating.set(currentHeatingStatus);
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
        DeviceState::instance().totalCycles.update([](uint32_t val) { return val + 1; });
        DeviceState::instance().sessionCycles.update([](uint32_t val) { return val + 1; });
        DeviceState::instance().totalDuration.update([durationMs](uint32_t val) { return val + durationMs; });

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
            DeviceState::instance().sessionClicks.set(doc["clicks"].as<int>());
        }
        if (!doc["caps"].isNull()) {
            DeviceState::instance().sessionCaps.set(doc["caps"].as<int>());
        }
        if (!doc["consumption"].isNull()) {
            DeviceState::instance().sessionConsumption.set(doc["consumption"].as<float>());
        }
        if (!doc["consumptionTotal"].isNull()) {
            DeviceState::instance().todayConsumption.set(doc["consumptionTotal"].as<float>());
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
        screenManager.setScreen(screenManager.getScreen(targetType), ScreenTransition::FADE);
        return true;
    }

    return false;
}

void Device::setCurrentCycle(int cycle) {
    lastSetCycle = cycle;
    Serial.printf("📊 Current cycle set to: %d\n", lastSetCycle);
}


