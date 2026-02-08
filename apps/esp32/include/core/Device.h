#pragma once

#include <Arduino.h>
#include <memory>
#include <functional> // For std::function

#include "Types.h"
#include "core/StateBinder.h"
#include "driver/net/Network.h"

class HeaterController;
class DeviceUI;

class Device {
    HeaterController heater;
    DeviceUI ui;
    Network network;

    void initNVS();

public:
    Device();
    void setup();
    void loop();
};
