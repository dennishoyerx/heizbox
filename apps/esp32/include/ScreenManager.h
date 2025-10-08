#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "Screen.h"
#include "InputManager.h"
#include "DisplayManager.h"
#include "ScreenType.h"

class ScreenManager {
private:
    Screen* currentScreen;
    DisplayManager& display;
    InputManager& input;
    bool dirty;
    ScreenType currentScreenType;

public:
    ScreenManager(DisplayManager& disp, InputManager& inp);
    void setScreen(Screen* newScreen);
    void update();
    void draw();
    void handleInput(InputEvent event);
    void setDirty() { dirty = true; }
    ScreenType getCurrentScreenType() const { return currentScreenType; }
};

#endif