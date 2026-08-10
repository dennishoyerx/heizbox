#pragma once
#include "Arduino.h"
#include "driver/net/WebSocketManager.h"

class Timer {
public:
    uint32_t start() { startMs = millis(); return startMs; };
    uint32_t stop()  { durationMs += millis() - startMs; startMs = 0; return durationMs; };
    void reset() { startMs = 0; durationMs = 0; };

    uint32_t getDuration() const { return getDurationMs() / 1000; }
    uint32_t getDurationMs() const { return durationMs + millis() - startMs; }
    uint32_t getStartMs() const { return startMs; }

protected:
    uint32_t durationMs;
    uint32_t startMs;
};

class HeatCycle {
public:
    uint32_t start();
    uint32_t stop();
    void submit();
    uint32_t getTimer() const { return timer.getDuration(); }
    uint32_t getTimerMs() const { return timer.getDurationMs(); }

private:
    Timer timer;
};
