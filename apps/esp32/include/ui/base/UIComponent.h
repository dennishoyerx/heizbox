#ifndef UICOMPONENT_H
#define UICOMPONENT_H

#include <cstdint>
#include <TFT_eSPI.h> // For uint16_t color



class UIComponent {
public:
    UIComponent(int16_t x, int16_t y, uint16_t color = TFT_WHITE);
    virtual ~UIComponent() = default;

    virtual void draw() = 0; // Pure virtual

    void setPosition(int16_t x, int16_t y);
    void setColor(uint16_t color);

protected:
    int16_t _x;
    int16_t _y;
    uint16_t _color;
};

#endif // UICOMPONENT_H
