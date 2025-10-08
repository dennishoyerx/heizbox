#pragma once

#include "Screen.h"
#include "DisplayManager.h"

class OtaUpdateScreen : public Screen {
public:
    OtaUpdateScreen(DisplayManager* displayManager);
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;

private:
    DisplayManager* _displayManager;
};
