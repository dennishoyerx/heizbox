// include/FireScreen.h
#ifndef FIRESCREEN_H
#define FIRESCREEN_H

#include "ui/base/Screen.h"
#include "HeaterController.h"
#include "ScreensaverScreen.h"
#include "StatsManager.h"
#include <functional>

class FireScreen : public Screen {
public:
    FireScreen(HeaterController& hc, ScreenManager* sm,
               ScreensaverScreen* ss, StatsManager* stm,
               std::function<void(int)> setCycleCb);

    void draw(DisplayDriver& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override { return ScreenType::FIRE; }

    void onEnter() override;
    void resetActivityTimer();

private:
    // Dependencies
    HeaterController& heater;
    ScreenManager* screenManager;
    ScreensaverScreen* screensaverScreen;
    StatsManager* statsManager;
    std::function<void(int)> setCycleCallback;

    // State
    struct {
        uint32_t heatingStartTime;
        uint32_t lastActivityTime;
        uint8_t currentCycle;
        bool showingSavedConfirmation;
        uint32_t confirmationStartTime;
        bool touchActive = false;
        uint32_t lastTouchStateChangeTime = 0;
        const uint32_t debounceDelay = 150; // milliseconds
        bool lastRawTouchActive = false;
        uint32_t lastTouchChangeTime = 0;
        int currentTouchValue = 0;
        const int baseTouchThreshold = 50; // This will be the base for touchOnThreshold
        int heaterInterferenceOffset = 150; // Empirically determined offset when heater is active
        const int touchOnThreshold = 50; // Value below which a touch is registered
        const int touchOffThreshold = 70; // Value above which a touch is considered released
    } state;

    // Cached values from state
    int cachedClicks;
    int cachedCaps;
    float cachedConsumption;
    float cachedTodayConsumption;

    // Helper methods
    void drawHeatingTimer(DisplayDriver& display);
    void drawStatus(DisplayDriver& display);
    void drawCycleInfo(DisplayDriver& display);
    void drawSessionStats(DisplayDriver& display);
    void handleCycleChange();
    void checkScreensaverTimeout();
    void _handleHeatingTrigger(bool shouldStartHeating);
};

#endif
