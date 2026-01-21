#pragma once

#include "ui/DeviceUI.h" // CONVERTED: Use forward.h
#include "DisplayDriver.h" // CONVERTED: Use forward.h
#include "heater/HeaterController.h" // CONVERTED: Use forward.h

/**
 * @brief Manages the binding of DeviceState observables to various hardware and system managers.
 *
 * This class provides static methods to synchronize DeviceState properties (like brightness,
 * dark mode, timezone, auto-stop time) with the corresponding manager classes.
 * It sets initial values and registers listeners for state changes to ensure real-time updates.
 */
class StateBinder {
public:
    static void bindDisplay(DisplayDriver* display);
    static void bindHeater(HeaterController* heater);
    static void bindDebug(DeviceUI* ui);

    static void bindAll(DeviceUI* ui, HeaterController* heater);
};
