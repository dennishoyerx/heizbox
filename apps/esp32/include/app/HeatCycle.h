#pragma once
#include "Arduino.h"
#include "net/WebSocketManager.h"

class Timer {
public:
    void start() { startedAt = millis(); };
    void stop()  { durationMs += millis() - startedAt; startedAt = 0; };
    void reset() { startedAt = 0; durationMs = 0; };

    uint16_t getDuration() const { return durationMs / 1000; }
    uint16_t getDurationMs() const { return durationMs; }
protected:
    uint32_t durationMs;
    uint32_t startedAt;
};

class HeatCycle {
public:
    HeatCycle();
    void start();
    void stop();
    void submit();
    uint16_t getTimer() const { return timer.getDuration(); }
    uint16_t getTimerMs() const { return timer.getDurationMs(); }

private:
    Timer timer;
};