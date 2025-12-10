#include "hardware/input/InputManager.h"
#include <Arduino.h>
#include "Config.h"


const InputManager::ButtonConfig InputManager::BUTTON_PINS[InputManager::NUM_BUTTONS] = {
    {HardwareConfig::JOY_UP_PIN, UP},
    {HardwareConfig::JOY_DOWN_PIN, DOWN},
    {HardwareConfig::JOY_LEFT_PIN, LEFT},
    {HardwareConfig::JOY_RIGHT_PIN, RIGHT},
    {HardwareConfig::JOY_PRESS_PIN, CENTER},
    {HardwareConfig::FIRE_BUTTON_PIN, FIRE}
};

InputManager::InputManager()
    : callback(nullptr) {
}

void InputManager::setup() {
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        pinMode(BUTTON_PINS[i].pin, INPUT_PULLUP);
    }
    Serial.println("🎮 InputManager initialized");
}

void InputManager::update() {
    const uint32_t now = millis();

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        const auto& cfg = BUTTON_PINS[i];
        const bool isLow = digitalRead(cfg.pin) == LOW;
        const bool wasPressed = isPressed(i);

        // --- State Change Detection ---
        if (isLow && !wasPressed && (now - lastDebounce[i] > InputConfig::DEBOUNCE_MS)) {
            // --- PRESS ---
            setPressed(i, true);
            setHoldSent(i, false);
            pressTimes[i] = now;
            lastDebounce[i] = now;
            if (callback) callback({PRESS, cfg.button});

        } else if (!isLow && wasPressed) {
            // --- RELEASE ---
            setPressed(i, false);
            lastDebounce[i] = now;

            // RELEASE nur senden, wenn HOLD getriggert wurde
            if (isHoldSent(i) && callback) {
               callback({RELEASE, cfg.button});
            } else  if (callback) {
                callback({PRESSED, cfg.button});
               // callback({RELEASE, cfg.button});
            }

            // Reset Hold-Flag, damit erneutes Halten wieder funktioniert
            setHoldSent(i, false);
        } else if (isLow && wasPressed && !isHoldSent(i) && (now - pressTimes[i] > InputConfig::HOLD_THRESHOLD_MS)) {
            // --- HOLD ---
            setHoldSent(i, true);
            lastHoldStep[i] = now;
            if (callback) callback({HOLD_ONCE, cfg.button});
            if (callback) callback({HOLD, cfg.button});
        } else if (isLow && wasPressed && isHoldSent(i)) {
            // --- HOLDING ---
            if (now - lastHoldStep[i] >= InputConfig::HOLDING_INTERVAL_MS) {
                lastHoldStep[i] = now;
                if (callback) callback({HOLD, cfg.button});
            }
        }
    }
}

void InputManager::setCallback(EventCallback cb) {
    callback = cb;
}


