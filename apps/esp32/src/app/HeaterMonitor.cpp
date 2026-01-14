#include "app/HeaterMonitor.h"
#include "heater/HeaterCycle.h"
#include "heater/HeaterState.h"
#include "core/DeviceState.h"
#include "utils/Logger.h"
#include "core/EventBus.h"
#include "app/HeatData.h"
#include "net/WebSocketManager.h"

HeaterMonitor::HeaterMonitor(HeaterController& heater): heater(heater) {
    EventBus::instance().subscribe<CycleFinishedData>(
        EventType::CYCLE_FINISHED, [this](const CycleFinishedData& d){
            heatCycleCompleted(d.duration / 1000);
            logPrint("x1");
        }
    );
}

void HeaterMonitor::heatCycleCompleted(uint32_t duration) {
    WebSocketManager::instance().sendHeatCycleCompleted(duration, HeaterCycle::current());
    HeaterCycle::next();
}

void HeaterMonitor::checkHeatingStatus() {
    const bool currentHeatingStatus = heater.isHeating();

    if (currentHeatingStatus != lastHeatingStatusSent) {
        DeviceState::instance().isHeating.set(currentHeatingStatus);
        WebSocketManager::instance().sendStatusUpdate(true, currentHeatingStatus);
        lastHeatingStatusSent = currentHeatingStatus;
    }
}
