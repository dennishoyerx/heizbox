// include/OtaUpdateScreen.h
#pragma once

#include "ui/base/Screen.h"
#include "DisplayDriver.h"

class OtaUpdateScreen : public Screen {
public:
    OtaUpdateScreen();
    void draw() override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override { return ScreenType::OTA_UPDATE; }

private:
    bool hasFailed = false;

};