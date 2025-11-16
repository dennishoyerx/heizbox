// core/Config.h
#pragma once

#include <Arduino.h>

struct HardwareConfig {
    static constexpr uint8_t TFT_BL_PIN = 4;
    static constexpr uint8_t STATUS_LED_PIN = 2;

    static constexpr uint8_t JOY_UP_PIN = 12;
    static constexpr uint8_t JOY_DOWN_PIN = 33;
    static constexpr uint8_t JOY_LEFT_PIN = 22;
    static constexpr uint8_t JOY_RIGHT_PIN = 14;
    static constexpr uint8_t JOY_PRESS_PIN = 15;

    static constexpr uint8_t FIRE_BUTTON_PIN = 13;

    static constexpr uint8_t HEATER_MOSFET_PIN = 32;

    static constexpr uint8_t THERMO_SCK_PIN = 25;
    static constexpr uint8_t THERMO_CS_PIN = 27;
    static constexpr uint8_t THERMO_SO_PIN = 26;

    static constexpr uint16_t SENSOR_TEMPERATURE_READ_INTERVAL_MS = 500;
};

struct NetworkConfig {
    static constexpr const char *BACKEND_WS_URL = "wss://backend.hzbx.de/ws";
    static constexpr const char *DEVICE_ID = "HeizboxESP32";
    static constexpr const char *NTP_SERVER = "pool.ntp.org";
    static constexpr const char *HOSTNAME = "Heizbox";
};

struct Timing {
    static constexpr uint32_t NVS_FLUSH_INTERVAL_MS = 5000;
    static constexpr uint32_t SCREENSAVER_TIMEOUT_MS = 600000;
    static constexpr uint32_t HEATCYCLE_TIMEOUT_MS = 30000;
};

