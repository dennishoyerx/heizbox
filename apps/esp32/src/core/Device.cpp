#include "core/Device.h"
#include "Config.h"
#include "credentials.h"
#include "driver/net/Network.h"
#include "ui/DeviceUI.h"
#include "heater/HeaterController.h"
#include "driver/Audio.h"
#include <SysModule.h>

#include <Wire.h>
#include <utility>

#include <Task.h>

Device::Device(): heater(), ui(heater), network() {
}

void Device::setup() {
    Serial.begin(115200);
    Wire.begin(HardwareConfig::SDA_PIN, HardwareConfig::SCL_PIN);

    if (!DebugFlags::LOG_BOOT) disableModuleLogging();

    initNVS();
    network.init(WIFI_SSID, WIFI_PASSWORD, NetworkConfig::HOSTNAME);
    heater.init();
    ui.init();

    StateBinder::bindAll(&ui, &heater);

    Audio::init();
    Audio::beepStartup();

    
    auto cb = [this]() {
        network.update();
    };

    auto task = dh::Task({
        .callback = cb,
        .config = {
            .name = "net",
            .stack_size_bytes = 4096,
            .priority = 0,
            .core_id = -1,
        }
    });
    task.start();


    Serial.println("✅ Device initialized");
}

void Device::loop() {
    heater.update();
    ui.update();
}


void Device::initNVS() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
}