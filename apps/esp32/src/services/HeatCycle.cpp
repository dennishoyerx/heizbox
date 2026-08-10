#include "services/HeatCycle.h"
#include "heater/HeaterCycle.h"

uint32_t HeatCycle::start() {
    WebSocketManager::instance().sendStatusUpdate(true, true);
    return timer.start();
}

uint32_t HeatCycle::stop() {
    WebSocketManager::instance().sendStatusUpdate(true, false);
    return timer.stop();
}

void HeatCycle::submit() {
    WebSocketManager::instance().sendHeatCycleCompleted(timer.getDuration(), HeaterCycle::current());
    HeaterCycle::next();
    timer.reset();
}