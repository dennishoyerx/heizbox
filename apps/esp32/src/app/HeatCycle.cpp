#include "app/HeatCycle.h"
#include "heater/HeaterCycle.h"

Timer::Timer() {}

void Timer::start() {
    startedAt = millis();
    WebSocketManager::instance().sendStatusUpdate(true, true);
}

void Timer::stop() {
    durationMs += millis() - startedAt;
    startedAt = 0;
    
    WebSocketManager::instance().sendStatusUpdate(true, stop);
}

void Timer::reset() {
    startedAt = 0;
    durationMs = 0;
}

HeatCycle::HeatCycle() {}

void HeatCycle::submit() {
    WebSocketManager::instance().sendHeatCycleCompleted(durationMs / 1000, HeaterCycle::current());
    HeaterCycle::next();
    reset();
}