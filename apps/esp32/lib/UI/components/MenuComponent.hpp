#pragma once
#include <RenderSurface.h>

class MenuComponent {};

void RRow(int width, int columns) {
    int col_width = width / columns;
    for (int i = 0; i < columns; i++) {
    }
};

using ColFn = std::function<void()>;
using RowCb = std::function<void(ColFn)>;

void Row(int width, RowCb cb) {
    int columns;

    cb([&columns]() {
        columns++;
    });
};


void R(RenderSurface s) {
    // w280 h240
    s.text(0, 0, "Heater");
    s.text(0, 0, "Display");
    s.text(0, 0, "Device");
    Row(280, [&s](ColFn col) {
        col();
        col();
        col();
    });
};