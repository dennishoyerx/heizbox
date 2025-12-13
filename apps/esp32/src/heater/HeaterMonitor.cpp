#include "heater/HeaterMonitor.h"
#include "heater/HeaterCycle.h"
#include "core/DeviceState.h"
#include "utils/Logger.h"
#include "core/EventBus.h"

HeaterMonitor::HeaterMonitor(HeaterController& heater, WebSocketManager& webSocket): 
    heater(heater),
    webSocket(webSocket) {
        
    EventBus::instance().subscribe<CycleFinishedData>(
        EventType::CYCLE_FINISHED, [this](const CycleFinishedData& d){
            heatCycleCompleted(d.duration / 1000);
        }
    );

}

void HeaterMonitor::heatCycleCompleted(uint32_t duration) {
    webSocket.sendHeatCycleCompleted(duration, HeaterCycle::current());
    HeaterCycle::next();
}

void HeaterMonitor::checkHeatingStatus() {
    const bool currentHeatingStatus = heater.isHeating();

    if (currentHeatingStatus != lastHeatingStatusSent) {
        DeviceState::instance().isHeating.set(currentHeatingStatus);
        WebSocketManager::getInstance()->sendStatusUpdate(true, currentHeatingStatus);
        lastHeatingStatusSent = currentHeatingStatus;
    }
}
