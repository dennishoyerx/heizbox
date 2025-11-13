// include/FireScreen.h
#ifndef FIRESCREEN_H
#define FIRESCREEN_H

#include "ui/base/Screen.h"
#include "hardware/heater/HeaterController.h"
#include "ScreensaverScreen.h"
#include "StatsManager.h"
#include <functional>

class FireScreen : public Screen {
public:
    FireScreen(HeaterController& hc, ScreenManager* sm,
               ScreensaverScreen* ss, StatsManager* stm);

    void draw(DisplayDriver& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override { return ScreenType::FIRE; }

    void onEnter() override;
    void resetActivityTimer();
    void onCycleFinalized();

private:
    // Dependencies
    HeaterController& heater;
    ScreenManager* screenManager;
    ScreensaverScreen* screensaverScreen;
    StatsManager* statsManager;

    // State
    struct {
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
    float cachedYesterdayConsumption;

    // Helper methods
    void drawHeatingTimer(TFT_eSprite* sprite);
    static void drawSessionRow(TFT_eSprite* sprite, const char* label, float consumption, int y, uint8_t bgColor, uint8_t borderColor, uint8_t textColor, bool invert = false, bool thin = false);
    void drawCycleInfo(DisplayDriver& display);
    void drawSessionStats(DisplayDriver& display);
    void handleCycleChange();
    void checkScreensaverTimeout();

public:
    void _handleHeatingTrigger(bool shouldStartHeating);
};

#endif
