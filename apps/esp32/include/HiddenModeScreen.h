// include/HiddenModeScreen.h
#ifndef HIDDENMODESCREEN_H
#define HIDDENMODESCREEN_H

#include "Screen.h"
#include "DisplayManager.h"
#include "ScreenType.h"

class HiddenModeScreen : public Screen {
private:
    unsigned long enterTime;
    bool fireButtonHeld;
    DisplayManager* displayManager;

public:
    HiddenModeScreen(DisplayManager* dm);
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
};

#endif
