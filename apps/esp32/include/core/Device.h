// include/Device.h
#pragma once

#include <Arduino.h>
#include <memory>

#include "hardware/HeaterController.h"
#include "hardware/InputManager.h"
#include "hardware/DisplayDriver.h"
#include "hardware/ClockManager.h"
#include "core/StatsManager.h"
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"

// UI
#include "ui/base/ScreenManager.h"
#include "ui/screens/FireScreen.h"
#include "ui/screens/HiddenModeScreen.h"
#include "ui/screens/ScreensaverScreen.h"
#include "ui/screens/OtaUpdateScreen.h"
#include "ui/screens/StatsScreen.h"
#include "ui/screens/TimezoneScreen.h"
#include "ui/screens/StartupScreen.h"
#include "ui/base/ScreenTransition.h" // New include

// Forward declarations
class GenericMenuScreen;

class Device {

public:

    Device();

    ~Device(); // Declare destructor

    void setup();

    void loop();



    void setCurrentCycle(int cycle);

private:
    // Core components
    InputManager input;
    HeaterController heater;
    DisplayDriver display;
    ClockManager clockManager;
    StatsManager statsManager;
    WiFiManager wifiManager;
    WebSocketManager webSocketManager;

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

    // State
    bool lastHeatingStatusSent = false;
    int lastSetCycle = 1;

    // Helper methods
    void setupScreenRegistry();
    void setupMainMenu();
    void setupOTA();
    void handleInput(InputEvent event);
    bool handleGlobalShortcuts(InputEvent event);
    void checkHeatingStatus();
    void checkHeatCycle();
    void handleWebSocketMessage(const char* type, const JsonDocument& doc);
};