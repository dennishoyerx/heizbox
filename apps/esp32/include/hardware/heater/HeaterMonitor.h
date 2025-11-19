// include/hardware/HeaterMonitor.h
#pragma once

#include "hardware/heater/HeaterController.h"
#include "net/WebSocketManager.h"
#include "core/StateManager.h"

/**
 * @brief Monitors the heater's status and manages heat cycle completion.
 *
 * This class is responsible for checking if the heater is active, updating the device state,
 * sending status updates via WebSocket, and processing completed heat cycles by updating
 * statistics and notifying the backend.
 */
class HeaterMonitor {
public:
    /**
     * @brief Constructs a HeaterMonitor instance.
     * @param heater Reference to the HeaterController instance.
     * @param webSocketManager Reference to the WebSocketManager instance.
     */
    HeaterMonitor(
        HeaterController& heater,
        WebSocketManager& webSocketManager
    );

    /**
     * @brief Checks the current heating status and sends updates if it has changed.
     */
    void checkHeatingStatus();

    /**
     * @brief Checks if a heat cycle has been completed and processes it.
     */
    void checkHeatCycle();

private:
    HeaterController& heater;
    WebSocketManager& webSocketManager;
    bool lastHeatingStatusSent = false;
};
