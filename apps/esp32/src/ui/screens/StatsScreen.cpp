// src/screens/StatsScreen.cpp
#include "core/DeviceState.h"
#include "ui/screens/StatsScreen.h"
#include "hardware/display/DisplayDriver.h"
#include "DisplayDriver.h"
#include "InputManager.h"
#include "ui/ColorPalette.h"
#include <TFT_eSPI.h>
#include "StateManager.h"

namespace {
    void centerText(DisplayDriver& display, int16_t y, const char* text, uint8_t color, uint8_t size) {
        int16_t x = (display.getTFTWidth() - display.getTextWidth(text, size)) / 2;
        display.drawText(x, y, text, color, size);
    }
}

StatsScreen::StatsScreen(StatsManager& sm) : stats(sm) {
    // State-Listener für Auto-Update
    DeviceState::instance().totalCycles.addListener([this](uint32_t) { markDirty(); });
    DeviceState::instance().sessionCycles.addListener([this](uint32_t) { markDirty(); });
}

void StatsScreen::draw(DisplayDriver& display) {
    display.clear();

    centerText(display, 10, "STATISTICS", COLOR_TEXT_PRIMARY, 2);

    auto& state = DeviceState::instance();

    // Total cycles
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Total: %lu cycles", state.totalCycles.get());
    display.drawText(30, 50, buffer, COLOR_TEXT_PRIMARY, 2);

    // Session cycles
    snprintf(buffer, sizeof(buffer), "Session: %lu cycles", state.sessionCycles.get());
    display.drawText(30, 80, buffer, COLOR_TEXT_PRIMARY, 2);

    // Average duration
    float avgDuration = state.totalCycles.get() > 0 
        ? state.totalDuration.get() / (float)state.totalCycles.get() / 1000.0f : 0.0f;
    snprintf(buffer, sizeof(buffer), "Avg: %.1f seconds", avgDuration);
    display.drawText(30, 110, buffer, COLOR_TEXT_PRIMARY, 2);

    // Instructions
    centerText(display, 180, "BACK: Hold CENTER", COLOR_TEXT_SECONDARY, 1);
}

void StatsScreen::update() {
    // No updates needed
}

void StatsScreen::handleInput(InputEvent event) {
    if (event.button == CENTER && event.type == HOLD) {
        // Will be handled by Device's global shortcuts
    }
}

ScreenType StatsScreen::getType() const {
    return ScreenType::STATS;
}
