#pragma once

#include <Arduino.h>
#include <memory>
#include <functional> // For std::function

#include "core/StateBinder.h"
#include "Types.h"
#include "core/EventBus.h"

#include "heater/HeaterController.h"
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"
#include "net/Network.h"
#include "net/OTASetup.h"
#include "heater/HeaterMonitor.h"


#include "ui/DeviceUI.h"

class Device {

public:
    Device();
    ~Device(); // Declare destructor
    void setup();
    void loop();

private:
    HeaterController heater;
    WiFiManager wifi;
    WebSocketManager webSocket;
    EventBus events;
    DeviceUI ui;

    std::unique_ptr<Network> network;
    std::unique_ptr<OTASetup> ota;
    std::unique_ptr<HeaterMonitor> heaterMonitor;
};
