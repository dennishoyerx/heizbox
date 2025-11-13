#pragma once

#include <hardware/display/DisplayDriver.h>

class UI {
public:
    UI(DisplayDriver* driver);
    TFT_eSprite* createSprite(int16_t w, int16_t h);

private:
    DisplayDriver* _driver;
};
