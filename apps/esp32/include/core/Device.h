// include/Device.h
#pragma once

#include <Arduino.h>
#include <memory>
#include <functional> // For std::function

#include "core/StateBinder.h"
#include "Types.h"
#include "core/EventBus.h"

#include "heater/HeaterController.h"
#include "hardware/input/InputManager.h"
#include "DisplayDriver.h"
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"
#include "net/Network.h"
#include "net/OTASetup.h"
#include "heater/HeaterMonitor.h"
#include "hardware/input/InputHandler.h"
#include "hardware/sensor/CapacitiveSensor.h"

// UI
#include "ui/base/ScreenManager.h"
#include "ui/UISetup.h"

class Device {

public:
    Device();
    ~Device(); // Declare destructor
    void setup();
    void loop();

private:
    InputManager input;
    HeaterController heater;
    std::unique_ptr<DisplayDriver> display;
    WiFiManager wifi;
    WebSocketManager webSocket;
    EventBus events;
    ScreenManager screenManager;

    std::unique_ptr<UISetup> uiSetup;
    std::unique_ptr<Network> network;
    std::unique_ptr<OTASetup> otaSetup;
    std::unique_ptr<HeaterMonitor> heaterMonitor;
    std::unique_ptr<InputHandler> inputHandler;
};
