// include/Device.h
#pragma once

#include <Arduino.h>
#include <memory>
#include <functional> // For std::function

#include "core/StateManager.h"
#include "core/StateBinder.h"
#include "core/Types.h"

#include "hardware/heater/HeaterController.h"
#include "hardware/input/InputManager.h"
#include "hardware/DisplayDriver.h"
#include "core/StatsManager.h"
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
#include "ui/screens/StatsScreen.h"
#include "ui/screens/TimezoneScreen.h"
#include "ui/screens/StartupScreen.h"
#include "ui/UISetup.h"


class Device {

public:
    Device();
    ~Device(); // Declare destructor
    void setup();
    void loop();
    void setCurrentCycle(int cycle);
    void onHeatCycleFinalized();

private:
    // Core components
    InputManager input;
    HeaterController heater;
    std::unique_ptr<DisplayDriver> display;
    StatsManager statsManager;
    WiFiManager wifiManager;
    WebSocketManager webSocketManager;
    CapacitiveSensor capacitiveSensor;

    // Screen management
    ScreenManager screenManager;

    // Screens
    FireScreen fireScreen;
    std::unique_ptr<GenericMenuScreen> mainMenuScreen;
    HiddenModeScreen hiddenModeScreen;
    ScreensaverScreen screensaverScreen;
    OtaUpdateScreen otaUpdateScreen;
    StatsScreen statsScreen;
    TimezoneScreen timezoneScreen;
    StartupScreen startupScreen;

    // UI Setup
    std::unique_ptr<UISetup> uiSetup;

    // Network Setup
    std::unique_ptr<Network> network;

    // OTA Setup
    std::unique_ptr<OTASetup> otaSetup;

    // Heater Monitor
    std::unique_ptr<HeaterMonitor> heaterMonitor;

    // Input Handler
    std::unique_ptr<InputHandler> inputHandler;

    // State
    int lastSetCycle = 1;

    // Helper methods
    void handleWebSocketMessage(const char* type, const JsonDocument& doc);

};
