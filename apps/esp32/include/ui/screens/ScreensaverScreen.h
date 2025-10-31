// include/ScreensaverScreen.h
#ifndef SCREENSAVERSCREEN_H
#define SCREENSAVERSCREEN_H

#include "ui/base/Screen.h"
#include "StateManager.h"
#include "hardware/DisplayDriver.h"
#include "utils/clock.h"

class ScreensaverScreen : public TimedScreen {
private:
    DisplayDriver* displayManager;
public:
    ScreensaverScreen(unsigned long timeout, DisplayDriver* dm, std::function<void()> callback = nullptr);
    
    void draw(DisplayDriver& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override { return ScreenType::SCREENSAVER; };
};

#endif
