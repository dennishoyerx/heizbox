#pragma once

#include "ui/base/Screen.h"
#include "ui/components/HeatUI.h"
#include "heater/HeaterController.h"
#include "ui/base/GenericMenuScreen.h"
#include <functional>

struct Consumption {
    float session;
    float today;
    float yesterday;
};

class FireScreen : public Screen {
public:
    FireScreen(HeaterController& hc);

    void draw() override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override { return ScreenType::FIRE; }
    void _handleHeatingTrigger(bool shouldStartHeating);

private:
    HeaterController& heater;
    std::unique_ptr<GenericMenuScreen> heaterMenuScreen;
    
    struct {
        Consumption consumption;
    } state;
};
