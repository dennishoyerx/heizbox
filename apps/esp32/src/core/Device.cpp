#include "core/Device.h"
#include "Config.h"
#include "credentials.h"
#include "core/DeviceState.h"
#include "net/Network.h"
#include "core/EventBus.h"
#include <utility>

Device::Device()
    : heater(), ui(heater), network(),
      heaterMonitor(std::make_unique<HeaterMonitor>(heater)) {}

Device::~Device() {}

void Device::setup() {
    Serial.begin(115200);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    heater.init();
    ui.init();
    network.init(WIFI_SSID, WIFI_PASSWORD, NetworkConfig::HOSTNAME);

    StateBinder::bindAll(&ui, &heater);
    
    Serial.println("✅ Device initialized");
}

void Device::loop() {
    heater.update();
    network.update();
    ui.update();

    heaterMonitor->checkHeatingStatus();
}
