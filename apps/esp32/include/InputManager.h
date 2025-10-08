#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <functional>
#include <Arduino.h>

// Pin definitions
#define FIRE_BUTTON_PIN     25
#define JOY_UP_PIN          12
#define JOY_DOWN_PIN        33
#define JOY_LEFT_PIN        27
#define JOY_RIGHT_PIN       14
#define JOY_PRESS_PIN       26

// InputEventType enum
enum InputEventType {
    PRESS,
    RELEASE,
    HOLD
};

// InputButton enum
enum InputButton {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CENTER,
    FIRE
};

// InputEvent struct
struct InputEvent {
    InputEventType type;
    InputButton button;
};

class InputManager {
private:
    typedef std::function<void(InputEvent)> EventCallback;
    EventCallback callback;

    // Button states
    bool upPressed, downPressed, leftPressed, rightPressed, centerPressed, firePressed;
    unsigned long upPressTime, downPressTime, leftPressTime, rightPressTime, centerPressTime, firePressTime;
    const unsigned long holdThreshold = 1000; // ms

    void checkHoldEvents();

public:
    InputManager();
    void init();
    void update();
    void setCallback(EventCallback cb);
};

#endif