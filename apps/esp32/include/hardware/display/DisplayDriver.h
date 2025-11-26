#pragma once

#include "core/Config.h"
#include "hardware/drivers/ITFTDriver.h"
#include "hardware/display/BacklightController.h"
#include <memory>

class ScreenManager;

class DisplayDriver {
public:
    explicit DisplayDriver(std::unique_ptr<ITFTDriver> tftDriver, 
                           std::unique_ptr<IBacklightController> backlightController);
    ~DisplayDriver();

    void init(ScreenManager* mgr);
    void clear();

    TFT_eSPI& getTFT() { return tft->getTFT(); }

    void setBrightness(uint8_t level);
    uint8_t getBrightness() const;

    int getTFTWidth() const { return DisplayConfig::WIDTH; }
    int getTFTHeight() const { return DisplayConfig::HEIGHT; }

private:
    std::unique_ptr<ITFTDriver> tft;
    std::unique_ptr<IBacklightController> backlight;
    
    ScreenManager* screenManager;
};
