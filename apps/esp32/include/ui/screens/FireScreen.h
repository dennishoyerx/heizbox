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
    void handleCycleChange(bool increment);
    void checkScreensaverTimeout();
};

#endif
