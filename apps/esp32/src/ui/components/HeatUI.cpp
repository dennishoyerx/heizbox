#include "ui\components\HeatUI.h"
#include "ui/base/Screen.h"
#include "ui/ColorPalette.h"

void HeatUI(RenderSurface s, uint32_t seconds) {
    uint8_t timerColor;
    if (seconds < 20) timerColor = COLOR_SUCCESS;
    else if (seconds < 35) timerColor = COLOR_WARNING;
    else if (seconds < 50) timerColor = COLOR_BLUE;
    else timerColor = COLOR_PURPLE;
    
    int centerX = 70;
    int centerY = 70;
    
    // === Vereinfachter Progress Ring ===
    int radius = 70;

    // Hintergrund-Ring
    s.sprite->fillCircle(centerX, centerY, radius, COLOR_BG);
    s.sprite->drawCircle(centerX, centerY, radius + 4, COLOR_TEXT_PRIMARY);

    float progress = min(progress, 1.0f);
    int endAngle = (int)(progress * 360);
    int startAngle = 180; 
    int stopAngle = startAngle + endAngle;

    s.sprite->drawArc(centerX, centerY,
                    radius + 7, radius - 7,
                    startAngle, stopAngle,
                    timerColor, COLOR_ACCENT, true);

    // === TIMER ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", seconds);
    
    s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
    s.sprite->setTextDatum(MC_DATUM);
    s.sprite->setTextSize(2);
    s.sprite->setFreeFont(&FreeSansBold18pt7b);
    s.sprite->drawString(timeStr, centerX, centerY, 1);
}