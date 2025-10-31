#ifndef UITEXT_H
#define UITEXT_H

#include "ui/base/UIComponent.h"
#include <string>

class UIText : public UIComponent {
public:
    UIText(int16_t x, int16_t y, const char* text, uint8_t size = 1, uint16_t color = TFT_WHITE);

    void draw() override;
    void setText(const char* text);
    void setSize(uint8_t size);

protected:
    const char* _text;
    uint8_t _size;
};

#endif // UITEXT_H
