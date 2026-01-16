#pragma once

#include <functional>
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "hardware/input/ButtonSource.h"
#define PCF8574_LOW_MEMORY
#include "PCF8574.h"

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

    void pcfInterrupt();

    static constexpr uint8_t NUM_BUTTONS = 6;

    struct ButtonConfig {
        uint8_t pin;
        InputButton button;
        ButtonSources source;
    };

    static const ButtonConfig BUTTON_PINS[NUM_BUTTONS];

private:
    PCF8574* pcf8574;
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