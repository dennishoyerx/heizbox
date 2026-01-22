#pragma once

#include "ITFTDriver.h"
#include "BacklightController.h"
#include <memory>

class ScreenManager;

class DisplayDriver {
public:
    explicit DisplayDriver(uint16_t width, uint16_t height, 
                           std::unique_ptr<ITFTDriver> tftDriver, 
                           std::unique_ptr<IBacklightController> backlightController);
    ~DisplayDriver();

    void init();
    void clear();

    TFT_eSPI& getTFT() { return tft->getTFT(); }

    void setBrightness(uint8_t level);
    uint8_t getBrightness() const;

    int getTFTWidth() const { return width; }
    int getTFTHeight() const { return height; }

    void setBackgroundColor(uint16_t color) { backgroundColor = color; }
    
    void setOrientation(uint8_t orientation);
    uint8_t getOrientation() const { return orientation; }

private:
    std::unique_ptr<ITFTDriver> tft;
    std::unique_ptr<IBacklightController> backlight;
    uint16_t backgroundColor = 0x00;
    uint16_t width;
    uint16_t height;
    uint8_t orientation;
};
