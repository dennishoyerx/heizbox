#ifndef STARTUPSCREEN_H
#define STARTUPSCREEN_H

#include "Screen.h"
#include "DisplayManager.h"
#include "ScreenType.h"
#include "InputManager.h"
#include <functional> // Include for std::function

class StartupScreen : public Screen {
private:
    unsigned long startTime;
    bool animationComplete;
    std::function<void()> onAnimationCompleteCallback; // Callback function

public:
    StartupScreen();
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
    void setOnAnimationCompleteCallback(std::function<void()> callback); // Setter for the callback
};

#endif