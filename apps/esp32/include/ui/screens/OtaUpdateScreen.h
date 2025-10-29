// include/OtaUpdateScreen.h
#pragma once

#include "ui/base/Screen.h"
#include "hardware/DisplayDriver.h"

class OtaUpdateScreen : public Screen {
public:
    OtaUpdateScreen(DisplayDriver* displayManager);
    void draw(DisplayDriver& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override { return ScreenType::OTA_UPDATE; }

private:
    DisplayDriver* _displayManager;
};