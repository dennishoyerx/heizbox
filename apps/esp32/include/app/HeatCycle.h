#pragma once
#include "Arduino.h"
#include "net/WebSocketManager.h"

class HeatCycle {
public:
    HeatCycle();

    void start();
    void pause();
    void resume();
    void submit();

private:
    uint32_t durationMs;
    uint32_t startedAt;
};