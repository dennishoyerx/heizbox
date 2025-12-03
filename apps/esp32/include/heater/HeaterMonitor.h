#pragma once

#include "heater/HeaterController.h"
#include "net/WebSocketManager.h"

class HeaterMonitor {
public:
    HeaterMonitor(
        HeaterController& heater,
        WebSocketManager& webSocket
    );

    void checkHeatingStatus();

    void checkHeatCycle();

private:
    HeaterController& heater;
    WebSocketManager& webSocket;
    bool lastHeatingStatusSent = false;
};
