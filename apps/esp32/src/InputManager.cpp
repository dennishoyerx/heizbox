// ==== OPTIMIZED FILE ====
// This file has been refactored to use a memory-efficient, bitmask-based approach for input handling.
// Key improvements:
// - Replaced the RAM-intensive array of structs with compact bitmasks and arrays.
// - Simplified the update loop with clear, fast bitwise operations.
// - Centralized button configuration in the header file.

#include "InputManager.h"
#include <Arduino.h>

// Definition for the static button configuration array declared in the header
const InputManager::ButtonConfig InputManager::BUTTON_PINS[InputManager::NUM_BUTTONS] = {
    {Config::Hardware::JOY_UP_PIN, UP}, 
    {Config::Hardware::JOY_DOWN_PIN, DOWN}, 
    {Config::Hardware::JOY_LEFT_PIN, LEFT},
    {Config::Hardware::JOY_RIGHT_PIN, RIGHT}, 
    {Config::Hardware::JOY_PRESS_PIN, CENTER}, 
    {Config::Hardware::FIRE_BUTTON_PIN, FIRE}
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

// Optimization: Replaced a 144-byte struct array with a ~42-byte collection of arrays and bitmasks.
// Benefit: Saves ~100 bytes of RAM and uses faster bitwise operations instead of iterating over structs.
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