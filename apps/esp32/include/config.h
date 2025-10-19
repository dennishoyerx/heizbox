// include/config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

namespace Config {
    // Device identification
    constexpr const char* DEVICE_ID = "my-esp32-device";
    constexpr const char* DEVICE_NAME = "Heizbox";

    // Backend URLs
    constexpr const char* BACKEND_WS_URL = "wss://heizbox-backend.dh19.workers.dev/ws";

    // Timing constants
    namespace Timing {
        constexpr uint32_t HEARTBEAT_INTERVAL_MS = 30000;
        constexpr uint32_t SCREENSAVER_TIMEOUT_MS = 30000;
        constexpr uint32_t NVS_FLUSH_INTERVAL_MS = 30000;
        constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 5000;
        constexpr uint32_t STATUS_CHECK_INTERVAL_MS = 1000;
    }

    // Hardware pins
    namespace Hardware {
        constexpr uint8_t LED_PIN = 2;
        constexpr uint8_t TFT_BL_PIN = 22;

        // Joystick
        constexpr uint8_t JOY_UP_PIN = 12;
        constexpr uint8_t JOY_DOWN_PIN = 33;
        constexpr uint8_t JOY_LEFT_PIN = 27;
        constexpr uint8_t JOY_RIGHT_PIN = 14;
        constexpr uint8_t JOY_PRESS_PIN = 25;

        // Fire button
        constexpr uint8_t FIRE_BUTTON_PIN = 25;

        // Heater
        constexpr uint8_t HEATER_MOSFET_PIN = 13;
        constexpr uint8_t STATUS_LED_PIN = 2;
    }

    // Display settings
    namespace Display {
        constexpr uint16_t WIDTH = 280;
        constexpr uint16_t HEIGHT = 240;
        constexpr uint8_t STATUS_BAR_HEIGHT = 50;
        constexpr uint8_t BRIGHTNESS_DEFAULT = 100;
        constexpr uint8_t BRIGHTNESS_MIN = 20;
        constexpr uint8_t BRIGHTNESS_MAX = 100;
    }

    // Heater settings
    namespace Heater {
        constexpr uint32_t DEFAULT_AUTOSTOP_MS = 120000;  // 2 minutes
        constexpr uint32_t COOLDOWN_DURATION_MS = 3000;
        constexpr uint32_t MIN_CYCLE_DURATION_MS = 10000;
    }
}

#endif
