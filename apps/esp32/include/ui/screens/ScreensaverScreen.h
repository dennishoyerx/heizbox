// include/ScreensaverScreen.h
#ifndef SCREENSAVERSCREEN_H
#define SCREENSAVERSCREEN_H

#include "ui/base/Screen.h"
#include "ClockManager.h"
#include "hardware/DisplayDriver.h"

class ScreensaverScreen : public TimedScreen {
private:
    ClockManager& clock;
    DisplayDriver* displayManager;

public:
    ScreensaverScreen(ClockManager& cm, unsigned long timeout, DisplayDriver* dm, std::function<void()> callback = nullptr);
    
    void draw(DisplayDriver& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override { return ScreenType::SCREENSAVER; };
};

#endif
