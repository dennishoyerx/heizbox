#ifndef STARTUPSCREEN_H
#define STARTUPSCREEN_H

#include "Screen.h"
#include "DisplayManager.h"
#include "ScreenType.h"
#include "InputManager.h"

class StartupScreen : public Screen {
private:
    unsigned long startTime;
    bool animationComplete;

public:
    StartupScreen();
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
};

#endif