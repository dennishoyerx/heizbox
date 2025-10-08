#ifndef FIRESCREEN_H
#define FIRESCREEN_H

#include "Screen.h"
#include "DisplayManager.h"
#include "HeaterController.h"
#include "ScreenManager.h"
#include "ScreenType.h"
#include "ScreensaverScreen.h"

class FireScreen : public Screen {
private:
    HeaterController& heater;
    ScreenManager* screenManager;
    ScreensaverScreen* screensaverScreen;
    unsigned long startTime;
    unsigned long elapsedTime;
    unsigned long lastActivityTime;

protected:
    void initState() override {
        setState("timer", 0);
        setState("temperature", 0);
        setState("active", 0);
    }

public:
    FireScreen(HeaterController& hc, ScreenManager* sm, ScreensaverScreen* ss);
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
    void resetActivityTimer();
};

#endif