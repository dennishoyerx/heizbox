// apps/esp32/include/hardware/drivers/ITFTDriver.h
#ifndef ITFTDRIVER_H
#define ITFTDRIVER_H

#include <cstdint>
#include <TFT_eSPI.h>

class ITFTDriver {
public:
    virtual ~ITFTDriver() = default;

    virtual void init() = 0;
    virtual void setRotation(uint8_t r) = 0;
    virtual void fillScreen(uint32_t color) = 0;
    virtual void setTextColor(uint16_t color, uint16_t bgcolor) = 0;
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void print(const char* text) = 0;
    virtual int16_t textWidth(const char* text) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) = 0;
    virtual TFT_eSPI& getTFT() = 0;
};

#endif // ITFTDRIVER_H
