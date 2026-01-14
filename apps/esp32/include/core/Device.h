#pragma once

#include <Arduino.h>
#include <memory>
#include <functional> // For std::function

#include "Types.h"
#include "core/StateBinder.h"
#include "heater/HeaterController.h"
#include "net/Network.h"
#include "ui/DeviceUI.h"

class Device {

public:
    Device();
    ~Device(); 
    void setup();
    void loop();

private:
    HeaterController heater;
    DeviceUI ui;
    Network network;
};
