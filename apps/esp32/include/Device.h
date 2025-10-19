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

    // Public methods for WebSocket
    void handleWebSocketEvent(WStype_t type, uint8_t * payload, size_t length);
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
    StartupScreen startupScreen;

    // --- Private Methods ---
    void handleInput(InputEvent event);
    void handleGlobalScreenSwitching(InputEvent event);
    void initWebSocket();
    void initWiFi();
    static void WiFiEvent(WiFiEvent_t event);

    // Optimization: Centralized JSON message sending to reduce code duplication and memory allocation.
    // Benefit: Reduces flash usage and avoids heap fragmentation from repeated String/JsonDocument creation.
    void sendJsonMessage(const char* type, bool includeIsOn = false, bool isOnValue = false, bool includeIsHeating = false, bool isHeatingValue = false);
    void sendHeatCycleCompleted(unsigned long duration, int cycle);

    // --- State Tracking ---
    bool _lastHeatingStatusSent = false; // Track last sent heating status
    unsigned long _lastHeartbeatTime = 0;
    int _lastSetCycle = 1; // Store the last set cycle, default to 1
};
