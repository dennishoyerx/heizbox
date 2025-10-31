// core/Config.h
#pragma once

#include <Arduino.h>

    struct HardwareConfig {
        static constexpr uint8_t TFT_BL_PIN = 22;
        static constexpr uint8_t STATUS_LED_PIN = 2;
        static constexpr uint8_t HEATER_MOSFET_PIN = 25;
        static constexpr uint8_t JOY_UP_PIN = 12;
        static constexpr uint8_t JOY_DOWN_PIN = 33;
        static constexpr uint8_t JOY_LEFT_PIN = 27;
        static constexpr uint8_t JOY_RIGHT_PIN = 14;
        static constexpr uint8_t JOY_PRESS_PIN = 26;
        static constexpr uint8_t FIRE_BUTTON_PIN = 13;
    };

    struct NetworkConfig {
        static constexpr const char* BACKEND_WS_URL = "wss://backend.hzbx.de/ws";
        static constexpr const char* DEVICE_ID = "HeizboxESP32";
        static constexpr const char* NTP_SERVER = "pool.ntp.org";

        // TODO: Add other network configurations here
    };

    struct Timing {
        static constexpr uint32_t NVS_FLUSH_INTERVAL_MS = 5000; // Flush NVS every 5 seconds
        static constexpr uint32_t SCREENSAVER_TIMEOUT_MS = 600000; // 30 seconds
    };

    // TODO: Add other configuration structs as needed
