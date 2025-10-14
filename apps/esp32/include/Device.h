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
#include <WebSocketsClient.h>

// Screens
#include "FireScreen.h"
#include "MainMenuScreen.h"
#include "HiddenModeScreen.h"
#include "ScreensaverScreen.h"
#include "OtaUpdateScreen.h"

#include "StatsScreen.h"
#include "TimezoneScreen.h"
#include "StartupScreen.h" // Include StartupScreen.h

class Device {
public:
    Device();

    void setup();
    void loop();

    // Public methods for WebSocket and static callbacks
    void handleWebSocketEvent(WStype_t type, uint8_t * payload, size_t length);
    static void WiFiEvent(WiFiEvent_t event);
    static Device* instance; 
    void sendHeatingStatus(bool heatingStatus);
    void setCurrentCycle(int cycle);


private:
    // Core components
    InputManager input;
    HeaterController heater;
    DisplayManager display;
    ClockManager clockManager;
    Preferences preferences;
    StatsManager statsManager;
    WebSocketsClient webSocket;

    // Screen management
    ScreenManager screenManager;

    // Screens
    FireScreen fireScreen;
    MainMenuScreen mainMenuScreen;
    HiddenModeScreen hiddenModeScreen;
    ScreensaverScreen screensaverScreen;
    OtaUpdateScreen otaUpdateScreen;
    StatsScreen statsScreen;
    TimezoneScreen timezoneScreen;
    StartupScreen startupScreen; // Add StartupScreen here

    void handleInput(InputEvent event);
    void handleGlobalScreenSwitching(InputEvent event);
    void initWebSocket();
    static void sendHeatingDataTask(void* pvParameters); // This can probably be removed later

    bool _lastHeatingStatusSent = false; // Track last sent heating status
    unsigned long _lastHeartbeatTime = 0;
    const unsigned long HEARTBEAT_INTERVAL = 30000; // 30 seconds
    int _lastSetCycle = 1; // Store the last set cycle, default to 1
};
