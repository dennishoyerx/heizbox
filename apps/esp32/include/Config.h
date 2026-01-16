#pragma once

#include <Arduino.h>

struct DebugFlags {
    static constexpr bool LOG_BOOT = false;
};

struct InputConfig {
    struct PCF8574 {
        static constexpr uint8_t SCL = 27;
        static constexpr uint8_t SDA = 26;
        static constexpr uint8_t INT_PIN = 25;
        static constexpr uint32_t UPDATE_MS = 200;
    };

    struct RotaryEncoder {
        static constexpr uint8_t CLK = 16;
        static constexpr uint8_t DT = 16;
        static constexpr uint8_t SW = 16;
    };


    static constexpr uint8_t FIRE_BUTTON = 16;

    
    static constexpr uint32_t HOLD_THRESHOLD_MS = 300;
    static constexpr uint32_t HOLDING_INTERVAL_MS = 150;
    static constexpr uint32_t DEBOUNCE_MS = 50;
};


struct HardwareConfig {
    static constexpr uint8_t SDA_PIN = 27;
    static constexpr uint8_t SCL_PIN = 26;
    
    static constexpr uint8_t SPEAKER_PIN = 25;
    static constexpr uint8_t TFT_BL_PIN = 16;
    static constexpr uint8_t STATUS_LED_PIN = 2;

    static constexpr uint8_t JOY_UP_PIN = 1;
    static constexpr uint8_t JOY_DOWN_PIN = 0; 
    static constexpr uint8_t JOY_LEFT_PIN = 2; 
    static constexpr uint8_t JOY_RIGHT_PIN = 3; 
    static constexpr uint8_t JOY_PRESS_PIN = 4;
    /*static constexpr uint8_t JOY_UP_PIN = 14;
    static constexpr uint8_t JOY_DOWN_PIN = 0; 
    static constexpr uint8_t JOY_LEFT_PIN = 21; 
    static constexpr uint8_t JOY_RIGHT_PIN = 19; 
    static constexpr uint8_t JOY_PRESS_PIN = 33;*/

    static constexpr uint8_t FIRE_BUTTON_PIN = 13;

    static constexpr uint8_t HEATER_MOSFET_PIN = 32;

    static constexpr uint8_t THERMO_SO_PIN = 0; // O
    static constexpr uint8_t THERMO_CS_PIN = 0; // Y
    static constexpr uint8_t THERMO_SCK_PIN = 0; // G 
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
    static constexpr uint16_t MAX_TEMPERATURE = 420;

    static constexpr uint32_t CYCLE_TIMEOUT = 120;
    static constexpr uint32_t CYCLE_TIMEOUT_MS = 120000;
    static constexpr uint32_t HEATCYCLE_MIN_DURATION_MS = 120000;

    struct IRSensor {};
    struct KSensor {
        static constexpr uint16_t READ_INTERVAL_MS = 220;
        static constexpr uint32_t OFF_TIME_MS = 0; // set to 200 when ktyp in use
    };
    struct ZVS {
        static constexpr uint32_t DUTY_CYCLE_PERIOD_MS = 1000; // 1 Sekunde pro Zyklus
    };
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
