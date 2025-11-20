#pragma once

#include "ui/base/Screen.h"
#include "ui/components/HeatUI.h"
#include "hardware/heater/HeaterController.h"
#include <functional>

struct Consumption {
    float session;
    float today;
    float yesterday;
};

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
        Consumption consumption;
        HeatState heater;
    } state;

    
    // Helper methods
    static void drawSessionRow(TFT_eSprite* sprite, const char* label, float consumption, int y, uint8_t bgColor, uint8_t borderColor, uint8_t textColor, bool invert = false, bool thin = false);

public:
    void _handleHeatingTrigger(bool shouldStartHeating);
};
