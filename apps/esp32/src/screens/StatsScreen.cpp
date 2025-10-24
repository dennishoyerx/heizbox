// src/screens/StatsScreen.cpp
#include "StatsScreen.h"
#include "DisplayManager.h"
#include "InputManager.h"
#include <TFT_eSPI.h>
#include "StateManager.h"

#define TFT_GRAY 0x7BEF

StatsScreen::StatsScreen(StatsManager& sm) : stats(sm) {
    // State-Listener für Auto-Update
    STATE.totalCycles.addListener([this](uint32_t) { markDirty(); });
    STATE.sessionCycles.addListener([this](uint32_t) { markDirty(); });
}

void StatsScreen::draw(DisplayManager& display) {
    display.clear(TFT_BLACK);

    centerText(display, 10, "STATISTICS", TFT_WHITE, 2);

    auto& state = STATE;

    // Total cycles
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Total: %lu cycles", state.totalCycles.get());
    display.drawText(30, 50, buffer, TFT_WHITE, 2);

    // Session cycles
    snprintf(buffer, sizeof(buffer), "Session: %lu cycles", state.sessionCycles.get());
    display.drawText(30, 80, buffer, TFT_WHITE, 2);

    // Average duration
    float avgDuration = state.totalCycles.get() > 0 
        ? state.totalDuration.get() / (float)state.totalCycles.get() / 1000.0f : 0.0f;
    snprintf(buffer, sizeof(buffer), "Avg: %.1f seconds", avgDuration);
    display.drawText(30, 110, buffer, TFT_WHITE, 2);

    // Instructions
    centerText(display, 180, "BACK: Hold CENTER", TFT_GRAY, 1);
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
