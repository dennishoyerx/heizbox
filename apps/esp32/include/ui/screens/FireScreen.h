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
    FireScreen(HeaterController& hc);

    void draw(DisplayDriver& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override { return ScreenType::FIRE; }

private:
    HeaterController& heater;

    // State
    struct {
        uint8_t currentCycle;

        bool isHeating = false;
        float targetTemp = 0;
        float currentTemp = 0;
        uint8_t power = 0;

        
        float consumption = 0;
        float todayConsumption = 0;
        float yesterdayConsumption = 0;
    } state;

    
    // Helper methods
    void drawHeatingTimer(TFT_eSprite* sprite, uint32_t seconds);
    static void drawSessionRow(TFT_eSprite* sprite, const char* label, float consumption, int y, uint8_t bgColor, uint8_t borderColor, uint8_t textColor, bool invert = false, bool thin = false);
    void drawCycleInfo(DisplayDriver& display);
    void drawSessionStats(DisplayDriver& display);

public:
    void _handleHeatingTrigger(bool shouldStartHeating);
};

#endif
