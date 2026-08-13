#pragma once

#include "driver/net/WiFiManager.h"
#include "driver/net/OTASetup.h"
#include "services/FirmwareUpdater.h"
#include "Config.h"
#include <ArduinoJson.h>
#include <functional>


class Network {
public:
    Network();

    void init(const char* ssid, const char* password, const char* hostname);
    void setupWifi(const char* ssid, const char* password, const char* hostname);
    void update();
    void handleWebSocketMessage(const char* type, const JsonDocument& doc);
    FirmwareUpdater& firmware() { return firmwareUpdater; }


private:
    WiFiManager wifi;
    OTASetup ota;
    FirmwareUpdater firmwareUpdater;

    bool initialized = false;
};
