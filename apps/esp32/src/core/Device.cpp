#include "core/Config.h"
#include "core/Device.h" 
#include "core/DeviceState.h" 
#include "credentials.h"
#include <ArduinoOTA.h>
#include "core/StateManager.h"
#include "utils/Logger.h"
#include "utils/Logs.h"
#include <utility> 
#include <WebServer.h>

// Network
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"
#include "net/Network.h"

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
#include "ui/UISetup.h"

#include "hardware/drivers/TFT_eSPI_Driver.h"
#include "hardware/display/BacklightController.h"

WebServer server(80);

Device::Device()
    : input(),
      heater(),
      display(std::make_unique<DisplayDriver>(
          std::make_unique<TFT_eSPI_Driver>(),
          std::make_unique<BacklightController>()
      )),
      statsManager(),
      wifiManager(),
      webSocketManager(),
      capacitiveSensor(heater, [this](bool start) { fireScreen._handleHeatingTrigger(start); }),
      screenManager(display.get(), input),
      fireScreen(heater, &screenManager, &screensaverScreen, &statsManager,
                 [this](int cycle) { this->setCurrentCycle(cycle); }),
      hiddenModeScreen(display.get()),
      screensaverScreen(DeviceState::instance().sleepTimeout.get(), display.get(), [this]() {
          fireScreen.resetActivityTimer();
          screenManager.setScreen(&fireScreen);
      }),
      otaUpdateScreen(otaUpdateScreen),
      statsScreen(statsManager),
      timezoneScreen(&screenManager),
      startupScreen([this]() {
          screenManager.setScreen(&fireScreen, ScreenTransition::FADE);
      }),
      lastSetCycle(1),
      mainMenuScreen(nullptr),
      uiSetup(std::make_unique<UISetup>(
          screenManager, fireScreen, hiddenModeScreen, screensaverScreen,
          otaUpdateScreen, statsScreen, timezoneScreen, startupScreen,
          [this](int cycle) { this->setCurrentCycle(cycle); }
      )),
      network(std::make_unique<Network>(
          wifiManager, webSocketManager,
          [this](const char* type, const JsonDocument& doc) { this->handleWebSocketMessage(type, doc); }
      )),
      otaSetup(std::make_unique<OTASetup>(screenManager, otaUpdateScreen, fireScreen)),
      heaterMonitor(std::make_unique<HeaterMonitor>(
          heater, webSocketManager, statsManager, lastSetCycle,
          [this]() { this->onHeatCycleFinalized(); }
      )),
      inputHandler(std::make_unique<InputHandler>(screenManager))
{}

Device::~Device() = default;

void Device::setup() {
    Serial.begin(115200);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Initialize core components
    display->init(&screenManager);
    input.init();
    heater.init();
    statsManager.init();

    // Setup screens
    uiSetup->setupScreenRegistry();
    screenManager.setScreen(&startupScreen);

    input.setCallback([this](InputEvent event) {
        inputHandler->handleInput(event);
    });

    StateBinder::bindAll(display.get(), &heater);
    DeviceState::instance().display = display.get();

    mainMenuScreen = uiSetup->setupMainMenu();

    network->setup(WIFI_SSID, WIFI_PASSWORD, "Heizbox");
    network->onReady([]() {
        static bool firmware_logged = false;
        if (!firmware_logged) {
            char firmwareInfo[64];
            snprintf(firmwareInfo, sizeof(firmwareInfo), "%s (%s)", FIRMWARE_VERSION, BUILD_DATE);
            logPrint("Firmware", firmwareInfo);
            firmware_logged = true;
        }
    });

    server.on("/log", [](){
        server.send(200, "text/plain", "Serial Log über WiFi!");
    });
    server.begin();
    
    otaSetup->setupOTA();

    Serial.println("✅ Device initialized");
}

void Device::loop() {
    server.handleClient();

    // Update managers
    wifiManager.update();
    webSocketManager.update();
    input.update();
    heater.update();
    statsManager.update();
    capacitiveSensor.update();

    // Update UI
    screenManager.update();
    screenManager.draw();
    display->renderStatusBar();

    heaterMonitor->checkHeatingStatus();
    heaterMonitor->checkHeatCycle();

    otaSetup->handleOTA();
}

// ============================================================================
// Helper Methods
// ============================================================================

void Device::onHeatCycleFinalized() {
    fireScreen.onCycleFinalized();
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

void Device::setCurrentCycle(int cycle) {
    lastSetCycle = cycle;
    Serial.printf("📊 Current cycle set to: %d\n", lastSetCycle);
}
