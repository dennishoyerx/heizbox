// src/Screen.cpp
#include "ui/base/Screen.h"
#include "hardware/display/DisplayDriver.h"
#include "ui/base/ScreenManager.h"
#include "ui/base/UI.h"



Screen::Screen() : manager(nullptr) {
    initState();
}

Screen::~Screen() {
}

void Screen::setManager(ScreenManager* mgr) {
    manager = mgr;
}

void Screen::setUI(UI* ui) {
    _ui = ui;
}

void Screen::redrawUI() {
    if (_ui) {
        _ui->forceRedraw();
    }
}

void Screen::clear() {
    if (_ui) {
        _ui->clear();
    }
}

void Screen::markDirty() {
    if (manager) {
        manager->setDirty();
    }
}

// Legacy
void Screen::centerText(DisplayDriver& display, int16_t y, const char* text,
                        uint16_t color, uint8_t size) {
    const int16_t textWidth = display.getTextWidth(text, size);
    const int16_t x = (display.getTFTWidth() - textWidth) / 2;
    display.drawText(x, y, text, color, size);
}

void Screen::onInput(InputButton button, InputEventType type, InputCallback callback) {
 //   manager->onInput(button, type, callback);
}

