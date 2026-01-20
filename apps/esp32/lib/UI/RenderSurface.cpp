#pragma once
#include "RenderSurface.h"
#include "Inter_12.h"
#include "Inter_16.h"
#include "Inter_18.h"
#include "Inter_24.h"
#include "Inter_22.h"
#include "Inter_20b.h"
#include "Inter_32.h"
#include "Inter_48.h"
#include "Inter_64b.h"
#include "Inter_16_bold.h"
#include "Inter_24_bold.h"
#include "Inter_32_bold.h"

const GFXfont* getFontForSize(TextSize ts) {
    switch (ts) {
        case TextSize::xs: return &Inter_16;
        case TextSize::sm: return &Inter_18;
        case TextSize::md: return &Inter_22;
        case TextSize::lg: return &Inter_32;
        case TextSize::xl: return &Inter_32;
        case TextSize::xxl: return &Inter_32;

        case TextSize::bsm: return &Inter_16B;
        case TextSize::bmd: return &Inter_22;
        case TextSize::blg: return &Inter_32B;
        case TextSize::bxl: return &Inter_48;
        case TextSize::bxxl: return &Inter_64B;
        default:           return &Inter_22;
    }
}
