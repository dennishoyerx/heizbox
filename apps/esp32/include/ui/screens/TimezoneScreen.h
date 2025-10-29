// include/TimezoneScreen.h
#ifndef TIMEZONESCREEN_H
#define TIMEZONESCREEN_H

#include "ui/base/Screen.h"
#include "ui/base/ScreenManager.h"
#include "ClockManager.h"
#include "CallbackMixin.h"

class TimezoneScreen : public Screen, protected CallbackMixin<> {
private:
    ClockManager& clockManager;
    ScreenManager* screenManager;
    int timezoneOffsetHours;

public:
    TimezoneScreen(ClockManager& cm, ScreenManager* sm);

    void draw(DisplayDriver& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
    void onEnter();
    
    using CallbackMixin::setCallback;
};

#endif