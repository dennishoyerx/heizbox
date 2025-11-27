#include "core/Device.h"
#include "Config.h"
#include "DisplayDriver.h"
#include "core/DeviceState.h"
#include "credentials.h"
#include "utils/Logger.h"
#include "utils/Logs.h"
#include <ArduinoOTA.h>
#include <utility>

// Network
#include "net/Network.h"
#include "net/WebSocketManager.h"
#include "net/WiFiManager.h"

// UI
#include "ui/UISetup.h"
#include "ui/base/ScreenManager.h"

#include "BacklightController.h"
#include "TFT_eSPI_Driver.h"

#include "core/EventBus.h"

Device::Device()
    : events(), input(), heater(),
      display(std::make_unique<DisplayDriver>(DisplayConfig::WIDTH, DisplayConfig::HEIGHT,
                                              std::make_unique<TFT_eSPI_Driver>(),
                                              std::make_unique<BacklightController>(HardwareConfig::TFT_BL_PIN))),
      wifi(), webSocket(), screenManager(*display, input),
      uiSetup(std::make_unique<UISetup>(screenManager, heater, display.get(), input)),
      network(std::make_unique<Network>(wifi, webSocket)),
      otaSetup(std::make_unique<OTASetup>(screenManager)),
      heaterMonitor(std::make_unique<HeaterMonitor>(heater, webSocket)),
      inputHandler(std::make_unique<InputHandler>(screenManager)) {}

Device::~Device() {}

void Device::setup() {
    Serial.begin(115200);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Initialize core components
    display->init();
    input.init();
    heater.init();

    // Setup screens
    uiSetup->setup();
    screenManager.switchScreen(ScreenType::STARTUP);

    input.setCallback([this](InputEvent event) { inputHandler->handleInput(event); });

    StateBinder::bindAll(display.get(), &heater);
    DeviceState::instance().display = display.get();

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
