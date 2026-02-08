#pragma once
#include "Arduino.h"
#include "driver/net/WebSocketManager.h"

class Timer {
public:
    uint16_t start() { return startMs = millis(); };
    uint16_t stop()  { return durationMs += millis() - startMs; startMs = 0; };
    void reset() { startMs = 0; durationMs = 0; };

    uint16_t getDuration() const { return getDurationMs() / 1000; }
    uint16_t getDurationMs() const { return durationMs + millis() - startMs; }
    uint16_t getStartMs() const { return startMs; }

protected:
    uint32_t durationMs;
    uint32_t startMs;
};

class HeatCycle {
public:
    uint16_t start();
    uint16_t stop();
    void submit();
    uint16_t getTimer() const { return timer.getDuration(); }
    uint16_t getTimerMs() const { return timer.getDurationMs(); }

private:
    Timer timer;
};