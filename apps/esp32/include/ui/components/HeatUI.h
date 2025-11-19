#pragma once

#include <TFT_eSPI.h>

class HeatUI {
public:
    HeatUI(uint32_t* seconds);
    void draw(TFT_eSprite* sprite, uint32_t seconds, bool isPaused);
private:
    uint32_t* seconds;
};