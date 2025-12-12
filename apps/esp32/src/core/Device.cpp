#include "core/Device.h"
#include "Config.h"
#include "core/DeviceState.h"
#include "credentials.h"
#include <utility>

// Network
#include "net/Network.h"
#include "net/WebSocketManager.h"
#include "net/WiFiManager.h"

#include "core/EventBus.h"

Device::Device()
    : heater(), ui(heater),
      wifi(), webSocket(), network(std::make_unique<Network>(wifi, webSocket)),
      ota(),
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

    StateBinder::bindAll(&ui, &heater);

    network->setup(WIFI_SSID, WIFI_PASSWORD, NetworkConfig::HOSTNAME);
    ota->setup();

    Serial.println("✅ Device initialized");
}

void Device::loop() {
    heater.update();
    network->update();
    ui.update();

    heaterMonitor->checkHeatingStatus();
    heaterMonitor->checkHeatCycle();

    ota->handle();
}
