#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

#include "HeaterController.h"
#include "InputManager.h"
#include "DisplayManager.h"
#include "ScreenManager.h"
#include "ClockManager.h"
#include "StatsManager.h"

// Screens
#include "FireScreen.h"
#include "MainMenuScreen.h"
#include "HiddenModeScreen.h"
#include "ScreensaverScreen.h"
#include "OtaUpdateScreen.h"

class Device {
public:
    Device();

    void setup();
    void loop();

private:
    // Core components
    InputManager input;
    HeaterController heater;
    DisplayManager display;
    ClockManager clockManager;
    Preferences preferences;
    StatsManager statsManager;

    // Screen management
    ScreenManager screenManager;

    // Screens
    FireScreen fireScreen;
    MainMenuScreen mainMenuScreen;
    HiddenModeScreen hiddenModeScreen;
    ScreensaverScreen screensaverScreen;
    OtaUpdateScreen otaUpdateScreen;

    void handleInput(InputEvent event);
    void handleGlobalScreenSwitching(InputEvent event);
    void sendHeatingData(unsigned long duration);
    static void WiFiEvent(WiFiEvent_t event);
    static void sendHeatingDataTask(void* pvParameters);

    // A static pointer to the device instance to access it in static callbacks
    static Device* instance; 
};
