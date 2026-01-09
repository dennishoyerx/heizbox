#pragma once

#include <functional>
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
/*
struct Button {
    uint8_t pin;
    InputButton button;
    ButtonSources source;
};

struct Buttons {
    Button up{HardwareConfig::JOY_UP_PIN, UP, GPIO};
    Button down{HardwareConfig::JOY_DOWN_PIN, DOWN, GPIO};
    Button left{HardwareConfig::JOY_LEFT_PIN, LEFT, GPIO};
    Button right{HardwareConfig::JOY_RIGHT_PIN, RIGHT, GPIO};
    Button center{HardwareConfig::JOY_PRESS_PIN, CENTER, GPIO};
    Button fire{HardwareConfig::FIRE_BUTTON_PIN, FIRE, GPIO};
};

enum ButtonSources {
    GPIO,
    PCF8574
};


class GpioButtonSource : public ButtonSource {
public:
    bool isPressed(uint8_t idx) override {
        return digitalRead(InputManager::BUTTON_PINS[idx].pin) == LOW;
    }
};

*/

class ButtonSource {
public:
    virtual bool isPressed(uint8_t index) = 0;
};

class Pcf8574ButtonSource : public ButtonSource {
public:
    Pcf8574ButtonSource(uint8_t addr) : address(addr) {}

    void begin() {
        Wire.begin(InputConfig::PCF8574::SDA, InputConfig::PCF8574::SCL);
    }

    void update() {
        Wire.requestFrom(address, (uint8_t)1);
        state = Wire.read();
    }

    bool isPressed(uint8_t idx) override {
        // active LOW → invertieren
        return !(state & (1 << idx));
    }

private:
    uint8_t address;
    uint8_t state = 0xFF;
};

enum InputEventType {
    PRESS,
    PRESSED,
    RELEASE,
    HOLD,
    HOLD_ONCE
};

enum InputButton {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CENTER,
    FIRE
};

struct InputEvent {
    InputEventType type;
    InputButton button;
};

class InputManager {
public:
    using EventCallback = std::function<void(InputEvent)>;

    InputManager();
    void setup();
    void update();
    void setCallback(EventCallback cb);

    static constexpr uint8_t NUM_BUTTONS = 6;

    struct ButtonConfig {
        uint8_t pin;
        InputButton button;
    };

    static const ButtonConfig BUTTON_PINS[NUM_BUTTONS];

private:
    Pcf8574ButtonSource* buttonSource;
    EventCallback callback = nullptr;

    uint8_t pressedMask = 0;
    uint8_t holdSentMask = 0;
    uint32_t pressTimes[NUM_BUTTONS] = {0};
    uint32_t lastDebounce[NUM_BUTTONS] = {0};
    uint32_t lastHoldStep[NUM_BUTTONS] = {0};

    // --- Inline helper functions for bitmask manipulation ---
    inline bool isPressed(uint8_t idx) const { return pressedMask & (1 << idx); }
    inline void setPressed(uint8_t idx, bool val) {
        if (val) pressedMask |= (1 << idx);
        else pressedMask &= ~(1 << idx);
    }
    inline bool isHoldSent(uint8_t idx) const { return holdSentMask & (1 << idx); }
    inline void setHoldSent(uint8_t idx, bool val) {
        if (val) holdSentMask |= (1 << idx);
        else holdSentMask &= ~(1 << idx);
    }
};
