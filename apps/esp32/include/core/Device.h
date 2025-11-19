// include/Device.h
#pragma once

#include <Arduino.h>
#include <memory>
#include <functional> // For std::function

#include "core/StateManager.h"
#include "core/StateBinder.h"
#include "core/Types.h"
#include "core/EventBus.h"

#include "hardware/heater/HeaterController.h"
#include "hardware/input/InputManager.h"
#include "hardware/display/DisplayDriver.h"
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"
#include "net/Network.h"
#include "net/OTASetup.h"
#include "hardware/heater/HeaterMonitor.h"
#include "hardware/input/InputHandler.h"
#include "hardware/sensor/CapacitiveSensor.h"

// UI
#include "ui/base/ScreenManager.h"
#include "ui/screens/FireScreen.h"
#include "ui/screens/HiddenModeScreen.h"
#include "ui/screens/ScreensaverScreen.h"
#include "ui/screens/OtaUpdateScreen.h"
#include "ui/screens/TimezoneScreen.h"
#include "ui/screens/StartupScreen.h"
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
    WiFiManager wifiManager;
    WebSocketManager webSocketManager;
    CapacitiveSensor capacitiveSensor;
    EventBus eventBus;
    ScreenManager screenManager;

    std::unique_ptr<UISetup> uiSetup;
    std::unique_ptr<Network> network;
    std::unique_ptr<OTASetup> otaSetup;
    std::unique_ptr<HeaterMonitor> heaterMonitor;
    std::unique_ptr<InputHandler> inputHandler;

    // Helper methods
    void handleWebSocketMessage(const char* type, const JsonDocument& doc);

};
