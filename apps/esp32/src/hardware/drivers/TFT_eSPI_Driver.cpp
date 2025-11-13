// apps/esp32/src/hardware/drivers/TFT_eSPI_Driver.cpp
#include "hardware/drivers/TFT_eSPI_Driver.h"

TFT_eSPI_Driver::TFT_eSPI_Driver() : tft() {}

void TFT_eSPI_Driver::init() {
    tft.init();
}

void TFT_eSPI_Driver::setRotation(uint8_t r) {
    tft.setRotation(r);
}

void TFT_eSPI_Driver::fillScreen(uint32_t color) {
    tft.fillScreen(color);
}

void TFT_eSPI_Driver::setTextColor(uint16_t color, uint16_t bgcolor) {
    tft.setTextColor(color, bgcolor);
}

void TFT_eSPI_Driver::setCursor(int16_t x, int16_t y) {
    tft.setCursor(x, y);
}

void TFT_eSPI_Driver::print(const char* text) {
    tft.print(text);
}

int16_t TFT_eSPI_Driver::textWidth(const char* text) {
    return tft.textWidth(text);
}

void TFT_eSPI_Driver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tft.drawRect(x, y, w, h, color);
}

void TFT_eSPI_Driver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tft.fillRect(x, y, w, h, color);
}

void TFT_eSPI_Driver::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
    tft.drawBitmap(x, y, bitmap, w, h, color);
}

void TFT_eSPI_Driver::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
    tft.drawXBitmap(x, y, bitmap, w, h, color);
}

TFT_eSPI& TFT_eSPI_Driver::getTFT() {
    return tft;
}
