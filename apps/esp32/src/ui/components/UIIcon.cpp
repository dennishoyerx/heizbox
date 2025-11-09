#include "ui/components/UIIcon.h"
#include "hardware/display/DisplayDriver.h"
#include "hardware/DisplayDriver.h"
#include "core/StateManager.h"

UIIcon::UIIcon(int16_t x, int16_t y, const uint8_t* bitmap, int16_t width, int16_t height, uint16_t color)
    : UIComponent(x, y, color), _bitmap(bitmap), _width(width), _height(height) {}

void UIIcon::draw(DisplayDriver& display) {
    // Assuming drawXBitmap is suitable for monochrome icons
    display.drawXBitmap(_x, _y, _bitmap, _width, _height, _color);
}

void UIIcon::setBitmap(const uint8_t* bitmap) {
    _bitmap = bitmap;
}
