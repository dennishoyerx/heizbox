#include "ui\components\Components.h"
#include "ui/base/Screen.h"
#include "ui/ColorPalette.h"

void Row(RenderSurface s, 
    const char* label, 
    float consumption, 
    int y, 
    uint8_t bgColor, 
    uint8_t borderColor, 
    uint8_t textColor, 
    bool invert, 
    bool thin) {
    int x = 0;
    int width = 250; 
    int height = thin ? 40 : 50;
    int radius = 16;
    uint8_t _bgColor;
    uint8_t _textColor;

    if (!invert) {
        _bgColor = bgColor;
        _textColor = textColor;
    } else {
        _bgColor = textColor;
        _textColor = bgColor;
    }

        s.sprite->fillSmoothRoundRect(x, y, width, height, radius, _bgColor, _textColor);

        // "Session" Text
        s.sprite->setTextSize(1);
        s.sprite->setTextColor(_textColor);
        s.sprite->setTextDatum(ML_DATUM);
        s.sprite->setFreeFont(thin ? &FreeSans9pt7b : &FreeSans12pt7b);
        s.sprite->drawString(label, 30, y + height / 2);

        // Verbrauchswert formatieren und anzeigen
        char consumptionStr[10];
        int integer = (int)consumption;
        int decimal = ((int)(consumption * 100 + 0.5f)) % 100;
        if (integer > 0)
        {
            sprintf(consumptionStr, "%d.%02dg", integer, decimal);
        }
        else
        {
            sprintf(consumptionStr, ".%02dg", decimal);
        }

        // Verbrauchswert rechts ausgerichtet
        s.sprite->setTextDatum(MR_DATUM);
        s.sprite->setFreeFont(thin ? &FreeSans9pt7b : &FreeSans18pt7b);
        s.sprite->drawString(consumptionStr, x + width - 12, y + height / 2);
}