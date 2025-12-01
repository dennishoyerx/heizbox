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

#include "core/EventBus.h"

Device::Device()
    : events(), heater(), ui(heater),
      wifi(), webSocket(), network(std::make_unique<Network>(wifi, webSocket)),
      otaSetup(std::make_unique<OTASetup>(ui)),
      heaterMonitor(std::make_unique<HeaterMonitor>(heater, webSocket)) {}

Device::~Device() {}

void Device::setup() {
    Serial.begin(115200);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Initialize core components
    heater.init();
    ui.setup();

    StateBinder::bindAll(ui.getDisplay(), &heater);
    DeviceState::instance().display = ui.getDisplay();

    network->setup(WIFI_SSID, WIFI_PASSWORD, NetworkConfig::HOSTNAME);
    otaSetup->setupOTA();

    Serial.println("✅ Device initialized");
}

void Device::loop() {
    network->update();
    ui.update();
    heater.update();
    //capacitiveSensor.update();

    heaterMonitor->checkHeatingStatus();
    heaterMonitor->checkHeatCycle();

    otaSetup->handleOTA();
}
