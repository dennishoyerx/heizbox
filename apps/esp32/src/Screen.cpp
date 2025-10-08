#include "Screen.h"
#include "ScreenManager.h"

Screen::Screen() : manager(nullptr) {}
Screen::~Screen() {}

void Screen::setState(const std::string& key, int value) {
    Serial.println("setState");

    auto it = state_.find(key);
    if (it == state_.end() || it->second != value) {
        state_[key] = value;
        if (manager) manager->setDirty();
    }
}

int Screen::getState(const std::string& key, int defaultValue) const {
    Serial.println("getState");
    auto it = state_.find(key);
    if (it != state_.end()) {
        return it->second;
    }
    return defaultValue;
}

void Screen::setManager(ScreenManager* mgr) {
    manager = mgr;
}