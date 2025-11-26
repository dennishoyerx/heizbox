// src/hardware/HeaterMonitor.cpp
#include "heater/HeaterMonitor.h"
#include "core/DeviceState.h"
#include "utils/Logger.h"

HeaterMonitor::HeaterMonitor(
    HeaterController& heater,
    WebSocketManager& webSocketManager
)
    : heater(heater),
      webSocketManager(webSocketManager)
{}

void HeaterMonitor::checkHeatingStatus() {
    const bool currentHeatingStatus = heater.isHeating();

    if (currentHeatingStatus != lastHeatingStatusSent) {
        DeviceState::instance().isHeating.set(currentHeatingStatus);
        webSocketManager.sendStatusUpdate(true, currentHeatingStatus);
        lastHeatingStatusSent = currentHeatingStatus;
    }
}

void HeaterMonitor::checkHeatCycle() {
    if (heater.isCycleFinished()) {
        const uint32_t durationMs = heater.getLastCycleDuration();
        const uint32_t durationSec = durationMs / 1000;

        // Update state
        uint8_t currentCycle = DeviceState::instance().currentCycle.update([](uint8_t val) { return val == 1 ? 2 : 1; });

/*        static uint8_t lastCycle = 0;
        if (currentCycle == 1) {
        }
*/
        heater.clearCycleFinishedFlag();

        // Send to backend
        webSocketManager.sendHeatCycleCompleted(durationSec, currentCycle);

        Serial.printf("✅ Heat cycle completed: %lu seconds (cycle %d)\n",
                     durationSec, currentCycle);
    }
}

