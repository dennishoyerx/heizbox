#include "app/HeatCycle.h"
#include "heater/HeaterCycle.h"

HeatCycle::HeatCycle() {}

void HeatCycle::start() {
    timer.start();
    WebSocketManager::instance().sendStatusUpdate(true, true);
}

void HeatCycle::stop() {
    timer.stop();
    WebSocketManager::instance().sendStatusUpdate(true, false);
}

void HeatCycle::submit() {
    WebSocketManager::instance().sendHeatCycleCompleted(timer.getDuration(), HeaterCycle::current());
    HeaterCycle::next();
    timer.reset();
}