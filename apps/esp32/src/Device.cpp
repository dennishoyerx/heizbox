// src/Device.cpp (Vereinfacht)
#include "Device.h"
#include "nvs_flash.h"
#include "credentials.h"
#include "config.h"
#include <ArduinoOTA.h>
#include "MenuBuilder.h"
#include "ScreenRegistry.h"
#include "ScreenBase.h"
#include <utility> // For std::move and std::make_unique

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
      screensaverScreen(clockManager, 30000, &display),
      otaUpdateScreen(&display),
      statsScreen(statsManager),
      timezoneScreen(clockManager, &screenManager),
      startupScreen(),
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
    clockManager.init();

    // Screen-Registry aufsetzen
    setupScreenRegistry();

    // Setup WiFi
    // WICHTIG: Menu erst nach Init der Manager erstellen!
    setupMainMenu();
    wifiManager.init(WIFI_SSID, WIFI_PASSWORD, "Heizbox");
    wifiManager.onConnectionChange([this](bool connected) {
        if (connected) {
            // WiFi connected - initialize WebSocket
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

    // Setup screens
    screenManager.setScreen(&startupScreen);

    startupScreen.setCallback([this]() {
        NAVIGATE_TO_WITH_TRANSITION(&screenManager, ScreenType::FIRE, ScreenTransition::FADE);
    });

    screensaverScreen.setCallback([this]() {
        fireScreen.resetActivityTimer();
        NAVIGATE_TO(&screenManager, ScreenType::FIRE);
    });

    input.setCallback([this](InputEvent event) {
        this->handleInput(event);
    });

    // Setup OTA
    setupOTA();

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
    // Brightness als int für Range-Item
    static int brightnessValue = display.getBrightness();

    // Temporäre Variable für Dark Mode, da isDarkMode() const ist
    static bool darkModeEnabled = display.isDarkMode();
    
    auto menuItems = MenuBuilder()
        .addRange("Brightness", &brightnessValue, 20, 100, 10, "%",
                 [this](int val) {
                     display.setBrightness(val);
                     display.saveSettings();
                 })
        
        .addToggle("Dark Mode", 
                  &darkModeEnabled, // Pass address of temporary variable
                  [this](bool enabled) {
                      display.toggleDarkMode(); // Call the actual toggle function
                  })
        
        .addAction("Timezone", [this]() {
            NAVIGATE_TO_WITH_TRANSITION(&screenManager, ScreenType::TIMEZONE, ScreenTransition::FADE);
        })
        
        .addAction("Statistics", [this]() {
            NAVIGATE_TO_WITH_TRANSITION(&screenManager, ScreenType::STATS, ScreenTransition::FADE);
        })
        
        .addAction("Reset Session", [this]() {
            statsManager.resetSession();
            Serial.println("📊 Session reset");
        })
        
        .build();
    
    mainMenuScreen = std::make_unique<GenericMenuScreen>("SETTINGS", std::move(menuItems));
    REGISTER_SCREEN(ScreenType::MAIN_MENU, *mainMenuScreen);
    
    // Setup timezone exit callback
    timezoneScreen.onExit([this]() {
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
        Serial.println("📲 OTA Update started");
        screenManager.setScreen(&otaUpdateScreen);
    });

    ArduinoOTA.onEnd([this]() {
        Serial.println("📲 OTA Update completed");
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

        // Send to backend
        webSocketManager.sendHeatCycleCompleted(durationSec, lastSetCycle);

        heater.clearCycleFinishedFlag();

        Serial.printf("✅ Heat cycle completed: %lu seconds (cycle %d)\n",
                     durationSec, lastSetCycle);
    }
}

void Device::handleWebSocketMessage(const char* type, const JsonDocument& doc) {
    if (strcmp(type, "sessionData") == 0 || strcmp(type, "sessionUpdate") == 0) {
        statsManager.updateSessionData(doc.as<JsonObjectConst>());
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
