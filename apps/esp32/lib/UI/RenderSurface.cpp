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

const GFXfont* ui::Text::getFontForSize(Text::Size ts) {
    switch (ts) {
        case Text::Size::xs: return &Inter_16;
        case Text::Size::sm: return &Inter_18;
        case Text::Size::md: return &Inter_22;
        case Text::Size::lg: return &Inter_32;
        case Text::Size::xl: return &Inter_32;
        case Text::Size::xxl: return &Inter_32;

        case Text::Size::bsm: return &Inter_16B;
        case Text::Size::bmd: return &Inter_22;
        case Text::Size::blg: return &Inter_32B;
        case Text::Size::bxl: return &Inter_48;
        case Text::Size::bxxl: return &Inter_64B;
        default:           return &Inter_22;
    }
}
