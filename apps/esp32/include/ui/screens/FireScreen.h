// include/FireScreen.h
#ifndef FIRESCREEN_H
#define FIRESCREEN_H

#include "ui/base/Screen.h"
#include "hardware/heater/HeaterController.h"
#include "ScreensaverScreen.h"
#include "StatsManager.h"
#include "hardware/sensor/TempSensor.h" // Include TempSensor
#include <functional>

class FireScreen : public Screen {
public:
    FireScreen(HeaterController& hc, ScreenManager* sm,
               ScreensaverScreen* ss, StatsManager* stm, TempSensor* ts);

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
    TempSensor* tempSensor; // TempSensor instance

    // State
    struct {
        uint32_t lastActivityTime;
        uint8_t currentCycle;
        bool showingSavedConfirmation;
        uint32_t confirmationStartTime;
        float targetTemp = 0;
        float currentTemp = 0;
    } state;

    // Cached values from state
    int cachedClicks;
    int cachedCaps;
    float cachedConsumption;
    float cachedTodayConsumption;
    float cachedYesterdayConsumption;

    
    unsigned long lastTempRead = 0;
    float lastTemp = NAN;

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
