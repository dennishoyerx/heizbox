// include/ScreensaverScreen.h
#ifndef SCREENSAVERSCREEN_H
#define SCREENSAVERSCREEN_H

#include "ScreenBase.h"
#include "ClockManager.h"

class ScreensaverScreen : public TimedScreen {
private:
    ClockManager& clock;
    DisplayManager* displayManager;

public:
    ScreensaverScreen(ClockManager& cm, unsigned long timeout, DisplayManager* dm, std::function<void()> callback = nullptr);
    
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
};

#endif
