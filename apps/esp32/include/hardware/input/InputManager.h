#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <functional>
#include <Arduino.h>
#include "core/Config.h"

enum InputEventType {
    PRESS,
    RELEASE,
    HOLD,
    HOLDING
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
    void init();
    void update();
    void setCallback(EventCallback cb);

private:
    static constexpr uint8_t NUM_BUTTONS = 6;

    struct ButtonConfig {
        uint8_t pin;
        InputButton button;
    };

    static const ButtonConfig BUTTON_PINS[NUM_BUTTONS];

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

#endif