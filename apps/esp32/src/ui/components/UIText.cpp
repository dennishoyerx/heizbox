#include "ui/components/UIText.h"
#include "core/StateManager.h"
#include "hardware/DisplayDriver.h"

UIText::UIText(int16_t x, int16_t y, const char* text, uint8_t size, uint16_t color)
    : UIComponent(x, y, color), _text(text), _size(size) {}

void UIText::draw(DisplayDriver& display) {
    display.drawText(_x, _y, _text, _color, _size);
}

void UIText::setText(const char* text) {
    _text = text;
}

void UIText::setSize(uint8_t size) {
    _size = size;
}
