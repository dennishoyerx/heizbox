#pragma once

#include "RenderSurface.h"

const GFXfont* getFontForSize(TextSize ts) {
    switch (ts) {
        case TextSize::sm: return &FreeSans9pt7b;
        case TextSize::md: return &FreeSans12pt7b;
        case TextSize::lg: return &FreeSans18pt7b;
        case TextSize::xl: return &FreeSans24pt7b;
        case TextSize::xxl: return &FreeSans18pt7b;

        case TextSize::bsm: return &FreeSansBold9pt7b;
        case TextSize::bmd: return &FreeSansBold12pt7b;
        case TextSize::blg: return &FreeSansBold18pt7b;
        case TextSize::bxl: return &FreeSansBold24pt7b;
        case TextSize::bxxl: return &FreeSansBold18pt7b;
        default:           return &FreeSans12pt7b;
    }
}
