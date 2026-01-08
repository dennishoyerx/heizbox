#include <Arduino.h>
#include "Config.h"

template <typename T>
class InputSource {
public:
    virtual bool isPressed(T b) = 0;
};

enum class JoystickButtons {
    Up, Down, Left, Right, Press
};
/*
class Joystick : public InputSource<JoystickButtons> {
public:
    Joystick();
    bool isPressed(JoystickButtons idx) override {
        return digitalRead(InputManager::BUTTON_PINS[idx].pin) == LOW;
    }
};*/