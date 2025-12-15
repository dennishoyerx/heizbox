#pragma once

#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"
#include "net/OTASetup.h"
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


private:
    WiFiManager wifi;
    OTASetup ota;

    bool initialized = false;
};
