#include "heater/HeatCycle.h"
#include "heater/HeaterCycle.h"

HeatCycle::HeatCycle() {}

void HeatCycle::pause() {
    durationMs += millis() - startedAt;
    startedAt = 0;
}

void HeatCycle::resume() {
    startedAt = millis();
}

void HeatCycle::submit() {
    WebSocketManager::instance().sendHeatCycleCompleted(durationMs / 1000, HeaterCycle::current());
    startedAt = 0;
    durationMs = 0;
    HeaterCycle::next();
}