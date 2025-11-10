#include "hardware/input/InputManager.h"
#include <Arduino.h>
#include "core/Config.h"



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

void InputManager::init() {
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
        if (isLow && !wasPressed && (now - lastDebounce[i] > DEBOUNCE_MS)) {
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
            // Optional: Uncomment to send RELEASE events
            // if (callback) callback({RELEASE, cfg.button});

        } else if (isLow && wasPressed && !isHoldSent(i) && (now - pressTimes[i] > HOLD_THRESHOLD_MS)) {
            // --- HOLD ---
            setHoldSent(i, true);
            if (callback) callback({HOLD, cfg.button});
        }
    }
}

void InputManager::setCallback(EventCallback cb) {
    callback = cb;
}


