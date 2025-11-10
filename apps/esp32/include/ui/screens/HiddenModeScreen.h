// include/HiddenModeScreen.h
#ifndef HIDDENMODESCREEN_H
#define HIDDENMODESCREEN_H

#include "ui/base/Screen.h"
#include "hardware/display/DisplayDriver.h"
#include "ui/base/ScreenTransition.h"

class HiddenModeScreen : public Screen {
private:
    unsigned long enterTime;
    bool fireButtonHeld;
    DisplayDriver* displayManager;

public:
    HiddenModeScreen(DisplayDriver* dm);
    void draw(DisplayDriver& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
};

#endif
