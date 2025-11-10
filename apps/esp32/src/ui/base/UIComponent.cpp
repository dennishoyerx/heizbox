#include "ui/base/UIComponent.h"
#include "hardware/display/DisplayDriver.h" // For DisplayDriver definition

UIComponent::UIComponent(int16_t x, int16_t y, uint16_t color)
    : _x(x), _y(y), _color(color) {}

void UIComponent::setPosition(int16_t x, int16_t y) {
    _x = x;
    _y = y;
}

void UIComponent::setColor(uint16_t color) {
    _color = color;
}
