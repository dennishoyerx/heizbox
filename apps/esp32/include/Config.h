// Config.h
#pragma once

#include <Arduino.h>

struct HardwareConfig {
    static constexpr uint8_t TFT_BL_PIN = 16;
    static constexpr uint8_t STATUS_LED_PIN = 2;

    static constexpr uint8_t JOY_UP_PIN = 14; // green
    static constexpr uint8_t JOY_DOWN_PIN = 12; // blue
    static constexpr uint8_t JOY_LEFT_PIN = 19; // purple
    static constexpr uint8_t JOY_RIGHT_PIN = 21; // white
    static constexpr uint8_t JOY_PRESS_PIN = 15; // orange

    static constexpr uint8_t FIRE_BUTTON_PIN = 13;

    static constexpr uint8_t HEATER_MOSFET_PIN = 32;

    static constexpr uint8_t THERMO_SO_PIN = 26; // O
    static constexpr uint8_t THERMO_CS_PIN = 27; // Y
    static constexpr uint8_t THERMO_SCK_PIN = 25; // G 

    static constexpr uint8_t IR_SDL_PIN = 33; // Purple
    static constexpr uint8_t IR_SDA_PIN = 22; // White
};

struct DisplayConfig {
    static constexpr uint16_t WIDTH = 280;
    static constexpr uint16_t HEIGHT = 240;
    static constexpr uint16_t STATUS_BAR_HEIGHT = 35;
    
    static constexpr uint8_t BRIGHTNESS_MIN = 20;
    static constexpr uint8_t BRIGHTNESS_MAX = 100;
    static constexpr uint8_t BRIGHTNESS_DEFAULT = 100;
};

struct HeaterConfig {
    static constexpr uint16_t SENSOR_TEMPERATURE_READ_INTERVAL_MS = 220;

    static constexpr uint32_t HEATCYCLE_MIN_DURATION_MS = 120000;
    static constexpr uint32_t DUTY_CYCLE_PERIOD_MS = 1000; // 1 Sekunde pro Zyklus
    static constexpr uint32_t SENSOR_OFF_TIME_MS = 200;
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
};


struct InputConfig {
    static constexpr uint32_t HOLD_THRESHOLD_MS = 300;
    static constexpr uint32_t HOLDING_INTERVAL_MS = 150;
    static constexpr uint32_t DEBOUNCE_MS = 50;
};


