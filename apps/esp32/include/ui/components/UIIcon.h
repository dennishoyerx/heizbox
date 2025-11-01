#ifndef UIICON_H
#define UIICON_H

#include "ui/base/UIComponent.h"

class UIIcon : public UIComponent {
public:
    UIIcon(int16_t x, int16_t y, const uint8_t* bitmap, int16_t width, int16_t height, uint16_t color = TFT_WHITE);

    void draw(DisplayDriver& display) override;
    void setBitmap(const uint8_t* bitmap);

protected:
    const uint8_t* _bitmap;
    int16_t _width;
    int16_t _height;
};

#endif // UIICON_H
