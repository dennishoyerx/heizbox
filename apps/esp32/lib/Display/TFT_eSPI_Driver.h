#ifndef TFT_ESPI_DRIVER_H
#define TFT_ESPI_DRIVER_H

#include "ITFTDriver.h"
#include <TFT_eSPI.h>

class TFT_eSPI_Driver : public ITFTDriver {
public:
    TFT_eSPI_Driver();

    void init() override;
    void setRotation(uint8_t r) override;
    void fillScreen(uint32_t color) override;
    void setTextColor(uint16_t color, uint16_t bgcolor) override;
    void setCursor(int16_t x, int16_t y) override;
    void print(const char* text) override;
    int16_t textWidth(const char* text) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) override;
    void drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) override;

    TFT_eSPI& getTFT() override;

private:
    TFT_eSPI tft;
};

#endif // TFT_ESPI_DRIVER_H
