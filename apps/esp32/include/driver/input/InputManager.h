#pragma once

#include <functional>
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "driver/input/ButtonSource.h"
#include <ESP32RotaryEncoder.h>
#define PCF8574_LOW_MEMORY
#include "PCF8574.h"

enum InputEventType {
    PRESS,
    PRESSED,
    RELEASE,
    HOLD,
    HOLD_ONCE,
    ROTARY_CW,      // Rotary Encoder im Uhrzeigersinn gedreht
    ROTARY_CCW,     // Rotary Encoder gegen Uhrzeigersinn gedreht
};

enum InputButton {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CENTER,
    FIRE,
    ROTARY_ENCODER
};

struct InputEvent {
    InputEventType type;
    InputButton button;
    int value = 0;  // Für Rotary Encoder: Delta-Wert oder absolute Position
};


class InputManager {
public:
    using EventCallback = std::function<void(InputEvent)>;

    InputManager();
    void setup();
    void update();
    void setCallback(EventCallback cb);

    void pcfInterrupt();

    static constexpr uint8_t NUM_BUTTONS = 7;

    struct ButtonConfig {
        uint8_t pin;
        InputButton button;
        ButtonSources source;
    };

    static const ButtonConfig BUTTON_PINS[NUM_BUTTONS];

private:
    PCF8574* pcf8574;
    RotaryEncoder* rotaryEncoder;

    EventCallback callback = nullptr;

    uint8_t pressedMask = 0;
    uint8_t holdSentMask = 0;
    uint32_t pressTimes[NUM_BUTTONS] = {0};
    uint32_t lastDebounce[NUM_BUTTONS] = {0};
    uint32_t lastHoldStep[NUM_BUTTONS] = {0};

    void onTurn(long value);

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