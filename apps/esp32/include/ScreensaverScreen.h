// include/ScreensaverScreen.h
#ifndef SCREENSAVERSCREEN_H
#define SCREENSAVERSCREEN_H

#include "Screen.h"
#include "DisplayManager.h"
#include "ClockManager.h"
#include "ScreenType.h"
#include "InputManager.h"
#include <functional>

class ScreensaverScreen : public Screen {
private:
    ClockManager& clock;
    unsigned long lastActivity;
    unsigned long sleepTimeout;
    DisplayManager* displayManager;
    std::function<void()> exitCallback;

public:
    ScreensaverScreen(ClockManager& cm, unsigned long timeout, DisplayManager* dm);
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
    void onExit(std::function<void()> callback);
};

#endif
