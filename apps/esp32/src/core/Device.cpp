#include "core/Config.h"
#include "core/Device.h" // Include Device.h
#include "credentials.h" // Include credentials for WiFi
#include <ArduinoOTA.h>
#include "core/StateManager.h"
#include "utils/Logger.h"
#include <utility> // For std::move and std::make_unique

// Hardware
#include "hardware/HeaterController.h"
#include "hardware/InputManager.h"
#include "hardware/DisplayDriver.h"
#include "hardware/ClockManager.h"
#include "hardware/OTASetup.h" // New include
#include "hardware/HeaterMonitor.h" // New include
#include "hardware/InputHandler.h" // New include

// Network
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"
#include "net/NetworkSetup.h" // New include

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
#include "ui/UISetup.h" // New include

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
      mainMenuScreen(nullptr), // Wird später initialisiert
      uiSetup(std::make_unique<UISetup>(
          screenManager, fireScreen, hiddenModeScreen, screensaverScreen,
          otaUpdateScreen, statsScreen, timezoneScreen, startupScreen,
          [this](int cycle) { this->setCurrentCycle(cycle); }
      )),
      networkSetup(std::make_unique<NetworkSetup>(
          wifiManager, webSocketManager,
          [this](const char* type, const JsonDocument& doc) { this->handleWebSocketMessage(type, doc); }
      )),
      otaSetup(std::make_unique<OTASetup>(screenManager, otaUpdateScreen, fireScreen)),
      heaterMonitor(std::make_unique<HeaterMonitor>(heater, webSocketManager, statsManager, lastSetCycle)),
      inputHandler(std::make_unique<InputHandler>(screenManager))
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

    // Setup screens
    uiSetup->setupScreenRegistry();
    screenManager.setScreen(&startupScreen);

    input.setCallback([this](InputEvent event) {
        inputHandler->handleInput(event);
    });

    clockManager.init();

    // State-Bindings erstellen
    StateBinder::bindAll(&display, &clockManager, &heater);

    // Setup Main Menu
    mainMenuScreen = uiSetup->setupMainMenu();

    // Setup Network
    networkSetup->setupNetwork(WIFI_SSID, WIFI_PASSWORD, "Heizbox");

    // Setup OTA
    otaSetup->setupOTA();

    logPrint("test");

    Serial.println("✅ Device initialized");
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
    heaterMonitor->checkHeatingStatus();

    // Check for completed heat cycles
    heaterMonitor->checkHeatCycle();

    // OTA handling
    otaSetup->handleOTA();

    delay(5);
}

// ============================================================================
// Helper Methods
// ============================================================================

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

void Device::setCurrentCycle(int cycle) {
    lastSetCycle = cycle;
    Serial.printf("📊 Current cycle set to: %d\n", lastSetCycle);
}

