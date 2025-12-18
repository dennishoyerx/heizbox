#include <Component.h>
#include "heater/HeaterState.h"
#include "ui/ColorPalette.h"
#include "bitmaps.h"
/*
struct IdleUI {
    Component* Temperature(Position pos, Dimension dim) {
        auto& hs = HeaterState::instance();

        return Component(pos, dim, [&hs](RenderSurface& s) {
            s.sprite->drawBitmap(-5, 0, image_temp_40, 40, 40, COLOR_TEXT_PRIMARY);
            s.text(30, 0, String(hs.temp), TextSize::lg);
            s.text(30, 30, String(hs.tempK), TextSize::lg);
            s.text(30, 60, String(hs.tempIR), TextSize::md);
        }).attachStates(hs.temp, hs.tempIR, hs.tempK);
    };

};

namespace FireUI {
    Component* Temperature(Position pos, Dimension dim) {
        auto& hs = HeaterState::instance();

        return Component(pos, dim, [&hs](RenderSurface& s) {
            s.sprite->drawBitmap(-5, 0, image_temp_40, 40, 40, COLOR_TEXT_PRIMARY);
            s.text(30, 0, String(hs.temp), TextSize::lg);
            s.text(30, 30, String(hs.tempK), TextSize::lg);
            s.text(30, 60, String(hs.tempIR), TextSize::md);
        }).attachStates(hs.temp, hs.tempIR, hs.tempK);
    };
};*/