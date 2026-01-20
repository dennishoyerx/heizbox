#pragma once

#include <Arduino.h>
#include <memory>
#include <functional> // For std::function

#include "Types.h"
#include "core/StateBinder.h"
#include "net/Network.h"

class HeaterController;
class DeviceUI;

class Device {
public:
    Device();
    void setup();
    void loop();

private:
    HeaterController heater;
    DeviceUI ui;
    Network network;
    void initNVS();
};
