#pragma once
#include "Arduino.h"
#include "net/WebSocketManager.h"

class Timer {
public:
    Timer();

    void start();
    void stop();
    void reset();

protected:
    uint32_t durationMs;
    uint32_t startedAt;
};

class HeatCycle: public Timer {
public:
    HeatCycle();
    void submit();
};