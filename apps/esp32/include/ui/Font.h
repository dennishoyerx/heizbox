// apps/esp32/include/ui/Font.h
#ifndef FONT_H
#define FONT_H

#include <cstdint>

class TFT_eSPI;

void setFont(uint16_t size, TFT_eSPI& renderer);

#endif // FONT_H
