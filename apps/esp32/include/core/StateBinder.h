#pragma once

#include "hardware/display/DisplayDriver.h"
#include "heater/HeaterController.h"

/**
 * @brief Manages the binding of DeviceState observables to various hardware and system managers.
 *
 * This class provides static methods to synchronize DeviceState properties (like brightness,
 * dark mode, timezone, auto-stop time) with the corresponding manager classes.
 * It sets initial values and registers listeners for state changes to ensure real-time updates.
 */
class StateBinder {
public:
    /**
     * @brief Binds the display brightness state to the DisplayDriver.
     * @param display Pointer to the DisplayDriver instance.
     */
    static void bindBrightness(DisplayDriver* display);

    /**
     * @brief Binds the dark mode state to the DisplayDriver.
     * @param display Pointer to the DisplayDriver instance.
     */
    static void bindDarkMode(DisplayDriver* display);

    /**
     * @brief Binds the auto-stop time state to the HeaterController.
     * @param heater Pointer to the HeaterController instance.
     */
    static void bindHeater(HeaterController* heater);

    /**
     * @brief Binds all relevant states to their respective managers.
     * @param display Pointer to the DisplayDriver instance.
     * @param clock Pointer to the ClockManager instance.
     * @param heater Pointer to the HeaterController instance.
     */
    static void bindAll(DisplayDriver* display, HeaterController* heater);
};
