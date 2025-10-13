#ifndef FIRESCREEN_H
#define FIRESCREEN_H

#include "Screen.h"
#include "DisplayManager.h"
#include "HeaterController.h"
#include "ScreenManager.h"
#include "ScreenType.h"
#include "ScreensaverScreen.h"

#include <functional> // Required for std::function

#include <functional> // Required for std::function

class FireScreen : public Screen {
private:
    HeaterController& heater;
    ScreenManager* screenManager;
    ScreensaverScreen* screensaverScreen;
    unsigned long startTime;
    unsigned long elapsedTime;
    unsigned long lastActivityTime;

    int _currentCycle;
    bool _showSavedConfirmation;
    unsigned long _savedConfirmationTime;
    std::function<void(int)> _setCycleCallback;

protected:
    void initState() override {
        setState("timer", 0);
        setState("temperature", 0);
        setState("active", 0);
    }

public:
    FireScreen(HeaterController& hc, ScreenManager* sm, ScreensaverScreen* ss, std::function<void(int)> setCycleCb);
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
    void resetActivityTimer();
};

#endif