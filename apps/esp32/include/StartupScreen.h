// include/StartupScreen.h
#ifndef STARTUPSCREEN_H
#define STARTUPSCREEN_H

#include "ScreenBase.h"

class StartupScreen : public AnimatedScreen {
public:
    StartupScreen();
    
    void draw(DisplayManager& display) override;
    void handleInput(InputEvent event) override;
    
    // Nutzt AnimatedScreen::setCallback statt eigener Methode
    using AnimatedScreen::setCallback;
};

#endif
