#include "ui\components\HeatUI.h"
#include "ui/base/Screen.h"
#include "ui/ColorPalette.h"
#include "bitmaps.h"

void HeatUI(RenderSurface s, HeatState state) {
    uint8_t timerColor;
    if (state.currentTemp < 160) timerColor = COLOR_BLUE; //COLOR_WARNING;
    else if (state.currentTemp < 170) timerColor = COLOR_SUCCESS;
    else if (state.progress < 180) timerColor = COLOR_WARNING;
    else if (state.progress < 190) timerColor = COLOR_ERROR;
    else timerColor = COLOR_PURPLE;

    int width = s.width();
    int height = s.height();
    
    int centerX = width / 2;
    int centerY = height / 2;
    int bottomY = centerY + height/2;
    int leftX = centerX - width/2;

    // progress = 0.0 .. 1.0
    int fillHeight = (int)(s.height() * state.progress);

    // Rechteck von unten nach oben
    s.sprite->fillRect(leftX, bottomY - fillHeight, width, fillHeight, timerColor);

    // === TIMER ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", state.elapsedSeconds);
    
    s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
    s.sprite->setTextDatum(MC_DATUM);
    s.sprite->setTextSize(2);
    s.sprite->setFreeFont(&FreeSansBold18pt7b);
    s.sprite->drawString(timeStr, centerX, centerY, 1);

    if (state.currentCycle == 1) {
        s.sprite->drawBitmap(width - 48 , 24, image_cap_fill_48, 48, 48, COLOR_TEXT_PRIMARY);
    }

    s.sprite->drawBitmap(0, 24, image_temp_48, 48, 48, COLOR_TEXT_PRIMARY);
    s.text(70, 40, isnan(state.currentTemp) ? "Err" : String(state.currentTemp), TextSize::xl);
    s.text(150, 40, String(state.targetTemp), TextSize::xl);

}