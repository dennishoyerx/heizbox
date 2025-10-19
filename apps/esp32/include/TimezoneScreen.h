// include/TimezoneScreen.h
#ifndef TIMEZONESCREEN_H
#define TIMEZONESCREEN_H

#include "Screen.h"
#include "DisplayManager.h"
#include "ScreenManager.h"
#include "ClockManager.h"
#include "InputManager.h"
#include <functional>

class TimezoneScreen : public Screen {
private:
    ClockManager& clockManager;
    ScreenManager* screenManager;
    Screen* mainMenuScreen;
    int timezoneOffsetHours;
    std::function<void()> exitCallback;

public:
    TimezoneScreen(ClockManager& cm, ScreenManager* sm);
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
    void onEnter();
    void setMainMenuScreen(Screen* screen);
    void onExit(std::function<void()> callback);
};

#endif