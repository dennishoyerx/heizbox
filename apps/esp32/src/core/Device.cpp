#include "core/Config.h"
#include "core/Device.h" 
#include "core/DeviceState.h" 
#include "hardware/display/DisplayDriver.h"
#include "credentials.h"
#include <ArduinoOTA.h>
#include "core/StateManager.h"
#include "utils/Logger.h"
#include "utils/Logs.h"
#include <utility> 

// Network
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"
#include "net/Network.h"

// UI
#include "ui/base/ScreenManager.h"
#include "ui/screens/FireScreen.h"
#include "ui/screens/OtaUpdateScreen.h"
#include "ui/screens/StartupScreen.h"
#include "ui/base/ScreenTransition.h"
#include "ui/UISetup.h"

#include "hardware/drivers/TFT_eSPI_Driver.h"
#include "hardware/display/BacklightController.h"

#include "core/EventBus.h"

Device::Device()
    : eventBus(),
        input(),
      heater(),
      display(std::make_unique<DisplayDriver>(
          std::make_unique<TFT_eSPI_Driver>(),
          std::make_unique<BacklightController>()
      )),
      wifiManager(),
      webSocketManager(),
      screenManager(*display, input),
      uiSetup(std::make_unique<UISetup>(
          screenManager, heater, display.get(), input
      )),
      capacitiveSensor(heater, [this](bool start) { uiSetup->getFireScreen()->_handleHeatingTrigger(start); }),
      network(std::make_unique<Network>(
          wifiManager, webSocketManager,
          [this](const char* type, const JsonDocument& doc) { this->handleWebSocketMessage(type, doc); }
      )),
      otaSetup(std::make_unique<OTASetup>(screenManager, *uiSetup->getOtaUpdateScreen(), *uiSetup->getFireScreen())),
      heaterMonitor(std::make_unique<HeaterMonitor>( heater, webSocketManager )),
      inputHandler(std::make_unique<InputHandler>(screenManager))
{}

Device::~Device() {}

void Device::setup() {
    Serial.begin(115200);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Initialize core components
    display->init(&screenManager);
    input.init();
    heater.init();

    // Setup screens
    uiSetup->setupScreens();
    screenManager.setScreen(uiSetup->getStartupScreen());

    input.setCallback([this](InputEvent event) {
        inputHandler->handleInput(event);
    });

    StateBinder::bindAll(display.get(), &heater);
    DeviceState::instance().display = display.get();

    uiSetup->setupMainMenu();

    network->setup(WIFI_SSID, WIFI_PASSWORD, NetworkConfig::HOSTNAME);
    network->onReady([]() {
        static bool firmware_logged = false;
        if (!firmware_logged) {
            char firmwareInfo[64];
            snprintf(firmwareInfo, sizeof(firmwareInfo), "%s (%s)", FIRMWARE_VERSION, BUILD_DATE);
            logPrint("Firmware", firmwareInfo);
            firmware_logged = true;
        }
    });
    
    otaSetup->setupOTA();

    Serial.println("✅ Device initialized");
}

void Device::loop() {
    network->update();
    input.update();
    heater.update();
    //capacitiveSensor.update();

    screenManager.update();
    screenManager.draw();

    heaterMonitor->checkHeatingStatus();
    heaterMonitor->checkHeatCycle();

    otaSetup->handleOTA();
}

// ============================================================================
// Helper Methods
// ============================================================================

void Device::handleWebSocketMessage(const char* type, const JsonDocument& doc) {
    if (strcmp(type, "sessionData") == 0 || strcmp(type, "sessionUpdate") == 0) {
        if (!doc["consumption"].isNull()) {
            DeviceState::instance().sessionConsumption.set(doc["consumption"].as<float>());
        }
        if (!doc["consumptionTotal"].isNull()) {
            DeviceState::instance().todayConsumption.set(doc["consumptionTotal"].as<float>());
        }
        if (!doc["consumptionTotal"].isNull()) {
            DeviceState::instance().yesterdayConsumption.set(doc["consumptionYesterday"].as<float>());
        }
        screenManager.setDirty();
    }
}


