// include/StartupScreen.h
#ifndef STARTUPSCREEN_H
#define STARTUPSCREEN_H

#include "ScreenBase.h"
#include "DisplayManager.h"
#include "InputManager.h"

class StartupScreen : public AnimatedScreen {
public:
    StartupScreen(std::function<void()> callback = nullptr);
    
    void draw(DisplayManager& display) override;
    void handleInput(InputEvent event) override;
};

#endif
