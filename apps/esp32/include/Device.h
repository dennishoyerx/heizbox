// include/Device.h
#pragma once

#include <Arduino.h>
#include <memory>
#include <Preferences.h>

#include "HeaterController.h"
#include "InputManager.h"
#include "DisplayManager.h"
#include "ScreenManager.h"
#include "ClockManager.h"
#include "StatsManager.h"
#include "WiFiManager.h"
#include "WebSocketManager.h"

// Screens
#include "FireScreen.h"
#include "HiddenModeScreen.h"
#include "ScreensaverScreen.h"
#include "OtaUpdateScreen.h"
#include "StatsScreen.h"
#include "TimezoneScreen.h"
#include "StartupScreen.h"

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
    DisplayManager display;
    ClockManager clockManager;
    StatsManager statsManager;
    WiFiManager wifiManager;
    WebSocketManager webSocketManager;
    Preferences preferences;

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