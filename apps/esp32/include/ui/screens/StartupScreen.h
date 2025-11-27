// include/StartupScreen.h
#ifndef STARTUPSCREEN_H
#define STARTUPSCREEN_H

#include "ui/base/Screen.h"
#include "DisplayDriver.h"
#include "hardware/input/InputManager.h"

class StartupScreen : public AnimatedScreen {
public:
    StartupScreen(std::function<void()> callback = nullptr);
    
    void draw() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override { return ScreenType::STARTUP; };
};

#endif
