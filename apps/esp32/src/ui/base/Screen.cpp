// src/Screen.cpp
#include "ui/base/Screen.h"
#include "hardware/display/DisplayDriver.h"
#include "ui/base/ScreenManager.h"
#include "ui/base/UI.h"



Screen::Screen() : manager(nullptr) {
    components = new Components();
    initState();
}

Screen::~Screen() {
    delete components;
}

void Screen::setState(const std::string& key, int value) {
    auto it = state_.find(key);
    if (it == state_.end() || it->second != value) {
        state_[key] = value;
        markDirty();
    }
}

int Screen::getState(const std::string& key, int defaultValue) const {
    auto it = state_.find(key);
    return (it != state_.end()) ? it->second : defaultValue;
}

void Screen::setManager(ScreenManager* mgr) {
    manager = mgr;
}

void Screen::setUI(UI* ui) {
    _ui = ui;
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


