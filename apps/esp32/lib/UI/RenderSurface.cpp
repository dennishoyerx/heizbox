#pragma once
#include "RenderSurface.h"
#include "Inter_12.h"
#include "Inter_16.h"
#include "Inter_18.h"
#include "Inter_24.h"
#include "Inter_20.h"
#include "Inter_20b.h"
#include "Inter_32.h"
#include "Inter_16_bold.h"
#include "Inter_24_bold.h"
#include "Inter_32_bold.h"

const GFXfont* getFontForSize(TextSize ts) {
    switch (ts) {
        case TextSize::sm: return &Inter_16;
        case TextSize::md: return &Inter_20;
        case TextSize::lg: return &Inter_24;
        case TextSize::xl: return &Inter_32;
        case TextSize::xxl: return &Inter_32;

        case TextSize::bsm: return &Inter_16B;
        case TextSize::bmd: return &Inter_20B;
        case TextSize::blg: return &Inter_24B;
        case TextSize::bxl: return &Inter_32B;
        case TextSize::bxxl: return &Inter_32B;
        default:           return &Inter_20;
    }
}
