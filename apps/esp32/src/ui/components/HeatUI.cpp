#include "ui\components\HeatUI.h"
#include "ui/base/Screen.h"
#include "ui/ColorPalette.h"

void HeatUI(RenderSurface s, uint32_t seconds, float progress, uint8_t currentTemp) {
    uint8_t timerColor;
    if (currentTemp < 160) timerColor = COLOR_WARNING;
    else if (currentTemp < 170) timerColor = COLOR_SUCCESS;
    else if (progress < 180) timerColor = COLOR_BLUE;
    else if (progress < 190) timerColor = COLOR_PURPLE;
    else timerColor = COLOR_ERROR;

    int centerX = 140;
    int centerY = 70;

    int bottomY = centerY + s.height()/2; // unterer Rand
    int leftX = centerX - s.width()/2;    // linker Rand

    // progress = 0.0 .. 1.0
    int fillHeight = (int)(s.height() * progress);

    // Rechteck von unten nach oben
    s.sprite->fillRect(leftX, bottomY - fillHeight, s.width(), fillHeight, timerColor);

    // === TIMER ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", seconds);
    
    s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
    s.sprite->setTextDatum(MC_DATUM);
    s.sprite->setTextSize(2);
    s.sprite->setFreeFont(&FreeSansBold18pt7b);
    s.sprite->drawString(timeStr, centerX, centerY, 1);
}