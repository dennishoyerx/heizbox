// src/hardware/HeaterMonitor.cpp
#include "hardware/HeaterMonitor.h"
#include "utils/Logger.h"

HeaterMonitor::HeaterMonitor(
    HeaterController& heater,
    WebSocketManager& webSocketManager,
    StatsManager& statsManager,
    int& lastSetCycle
)
    : heater(heater),
      webSocketManager(webSocketManager),
      statsManager(statsManager),
      lastSetCycle(lastSetCycle)
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

        // Add to stats
        statsManager.addCycle(durationMs);

        // Update state
        DeviceState::instance().totalCycles.update([](uint32_t val) { return val + 1; });
        DeviceState::instance().sessionCycles.update([](uint32_t val) { return val + 1; });
        DeviceState::instance().totalDuration.update([durationMs](uint32_t val) { return val + durationMs; });

        heater.clearCycleFinishedFlag();

        // Send to backend
        webSocketManager.sendHeatCycleCompleted(durationSec, lastSetCycle);

        Serial.printf("✅ Heat cycle completed: %lu seconds (cycle %d)\n",
                     durationSec, lastSetCycle);
    }
}

