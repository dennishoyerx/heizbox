#include "ui\components\HeatUI.h"
#include "ui/ColorPalette.h"


HeatUI::HeatUI(uint32_t* seconds) {
    this->seconds = seconds;
}

void HeatUI::draw(TFT_eSprite* sprite, uint32_t seconds, bool isPaused) {
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
    sprite->fillCircle(centerX, centerY, radius, COLOR_BG);
    sprite->drawCircle(centerX, centerY, radius + 4, COLOR_TEXT_PRIMARY);

    float progress = (float)seconds / 60.0f;
    int endAngle = (int)(progress * 360);
    int startAngle = 180; 
    int stopAngle = startAngle + endAngle;

    sprite->drawArc(centerX, centerY,
                    radius + 7, radius - 7,
                    startAngle, stopAngle,
                    timerColor, COLOR_ACCENT, true);

    // === TIMER ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", seconds % 60);
    
    sprite->setTextColor(COLOR_TEXT_PRIMARY);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(2);
    sprite->setFreeFont(&FreeSansBold18pt7b);
    sprite->drawString(timeStr, centerX, centerY, 1);
    
    
    // "HEIZT" or "PAUSE" Badge
    const char* badgeText = isPaused ? "PAUSE" : "HEIZT";
    uint16_t badgeColor = isPaused ? COLOR_WARNING : timerColor;
    sprite->fillRoundRect(centerX - 35, centerY + 50, 70, 20, 10, 0x8410);
    sprite->fillCircle(centerX - 20, centerY + 60, 3, badgeColor);
    sprite->setFreeFont(&FreeSans18pt7b);
    sprite->setTextSize(1);
    sprite->drawString(badgeText, centerX + 5, centerY + 60, 2);
    
    // Click Zone
    if (seconds >= 30 && seconds <= 50) {
        sprite->setFreeFont(&FreeSans18pt7b);
        sprite->setTextSize(1);
        sprite->setTextColor(COLOR_BG);
        sprite->drawString("CLICK ZONE", centerX, centerY + 80, 2);
    }
}