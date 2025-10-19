// Optimization: Replaced #define with constexpr for type safety and to prevent macro-related bugs.
// Benefit: Improved code robustness and clarity. Namespacing avoids global scope pollution.
#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

namespace Config {
    // --- Device Identification & Backend ---
    // Provides a unique identifier for the device and the WebSocket endpoint.
    constexpr const char* DEVICE_ID = "my-esp32-device";
    constexpr const char* BACKEND_WS_URL = "wss://heizbox-backend.dh19.workers.dev/ws";

    // --- Timing Configuration ---
    // Groups all critical timing values for easy tuning and better readability.
    namespace Timing {
        constexpr uint32_t HEARTBEAT_INTERVAL_MS = 30000;      // Interval for sending keep-alive messages.
        constexpr uint32_t SCREENSAVER_TIMEOUT_MS = 30000;     // Inactivity period before the screensaver activates.
        constexpr uint32_t NVS_FLUSH_INTERVAL_MS = 30000;        // Interval to batch-write statistics to flash memory.
    }

    // --- Hardware Pin Definitions ---
    // Centralizes hardware pin assignments for different board layouts.
    namespace Hardware {
        // Input Pins
        constexpr uint8_t JOY_UP_PIN = 12;
        constexpr uint8_t JOY_DOWN_PIN = 33;
        constexpr uint8_t JOY_LEFT_PIN = 27;
        constexpr uint8_t JOY_RIGHT_PIN = 14;
        constexpr uint8_t JOY_PRESS_PIN = 26;
        constexpr uint8_t FIRE_BUTTON_PIN = 25;

        // Output Pins
        // Optimization: Renamed to avoid conflicts with macros defined in libraries.
        constexpr uint8_t HEATER_MOSFET_PIN = 23; // Controls the induction heating coil.
        constexpr uint8_t STATUS_LED_PIN = 2;       // General purpose indicator LED.

        // Display & Peripherals
        constexpr uint8_t TFT_CS_PIN = 5;
        constexpr uint8_t TFT_DC_PIN = 4;
        constexpr uint8_t TFT_RST_PIN = 15;
        constexpr uint8_t TFT_BL_PIN = 22; // Backlight control pin
    }
}

#endif // CONFIG_H
