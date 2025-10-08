#include "InputManager.h"
#include <Arduino.h>

const unsigned long debounceDelay = 50; // ms

struct ButtonState {
    uint8_t pin;
    InputButton button;
    bool pressed;
    bool holdSent;
    unsigned long pressTime;
    unsigned long lastDebounce;
};

ButtonState buttons[] = {
    {JOY_UP_PIN,     UP,     false, false, 0, 0},
    {JOY_DOWN_PIN,   DOWN,   false, false, 0, 0},
    {JOY_LEFT_PIN,   LEFT,   false, false, 0, 0},
    {JOY_RIGHT_PIN,  RIGHT,  false, false, 0, 0},
    {JOY_PRESS_PIN,  CENTER, false, false, 0, 0},
    {FIRE_BUTTON_PIN,FIRE,   false, false, 0, 0}
};

InputManager::InputManager()
    : callback(nullptr)
{}

void InputManager::init() {
    for (auto& btn : buttons) {
        pinMode(btn.pin, INPUT_PULLUP);
    }
    Serial.println("🎮 InputManager initialized");
}

void InputManager::update() {
    unsigned long now = millis();
    for (auto& btn : buttons) {
        bool current = digitalRead(btn.pin) == LOW;

        // PRESS
        if (current && !btn.pressed && (now - btn.lastDebounce > debounceDelay)) {
            btn.pressed = true;
            btn.pressTime = now;
            btn.lastDebounce = now;
            btn.holdSent = false;
            if (callback) callback({ PRESS, btn.button });
        }
        // RELEASE
        else if (!current && btn.pressed) {
            btn.pressed = false;
            btn.lastDebounce = now;
            btn.holdSent = false;
            //if (callback) callback({ RELEASE, btn.button });
        }
        // HOLD
        else if (current && btn.pressed && !btn.holdSent && (now - btn.pressTime > holdThreshold)) {
            btn.holdSent = true;
            if (callback) callback({ HOLD, btn.button });
        }
    }
}

void InputManager::setCallback(EventCallback cb) {
    callback = cb;
}