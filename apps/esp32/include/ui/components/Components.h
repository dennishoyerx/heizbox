#pragma once

#include "ui/base/Screen.h"

//void IconText(RenderSurface s, String text, const uint8_t *icon, int16_t iconW, int16_t iconH);


void Row(RenderSurface s, 
    const char* label, 
    float consumption, 
    int y, 
    uint8_t bgColor, 
    uint8_t borderColor, 
    uint8_t textColor, 
    bool invert, 
    bool thin);
