// src/screens/FireScreen.cpp
#include "FireScreen.h"
#include "ScreenManager.h"
#include "bitmaps.h"
#include <TFT_eSPI.h>

FireScreen::FireScreen(HeaterController& hc, ScreenManager* sm,
                       ScreensaverScreen* ss, StatsManager* stm,
                       std::function<void(int)> setCycleCb)
    : heater(hc),
      screenManager(sm),
      screensaverScreen(ss),
      statsManager(stm),
      setCycleCallback(std::move(setCycleCb))
{
    state.heatingStartTime = 0;
    state.lastActivityTime = millis();
    state.currentCycle = 1;
    state.showingSavedConfirmation = false;
    state.confirmationStartTime = 0;
}

void FireScreen::onEnter() {
    resetActivityTimer();
}

void FireScreen::draw(DisplayManager& display) {
    display.clear(0x885);  // Dark gray background

    drawHeatingTimer(display);
    drawStatus(display);
    drawCycleInfo(display);
    drawSessionStats(display);
}

void FireScreen::drawHeatingTimer(DisplayManager& display) {
    if (!heater.isHeating()) return;

    const uint32_t elapsed = millis() - state.heatingStartTime;
    const uint32_t seconds = elapsed / 1000;

    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%02lu", seconds % 60);

    // Zentrierter großer Timer
    centerText(display, 80, timeStr, TFT_WHITE, 4);
}

void FireScreen::drawStatus(DisplayManager& display) {
    const char* status = nullptr;

    switch (heater.getState()) {
        case HeaterController::State::HEATING:   status = nullptr; break;  // Timer zeigt Status
        case HeaterController::State::COOLDOWN:  status = "COOLDOWN"; break;
        case HeaterController::State::ERROR:     status = "ERROR"; break;
        default:                     status = nullptr; break;
    }

    if (status) {
        //centerText(display, 180, status, TFT_WHITE, 2);
    }
}

void FireScreen::drawCycleInfo(DisplayManager& display) {
    char text[20];
    snprintf(text, sizeof(text), "Cycle: %d", state.currentCycle);
    display.drawBitmap(5, 30, epd_bitmap_fire, 32, 32, TFT_WHITE);
    display.drawText(15, 30, text, TFT_WHITE, 3);
}

void FireScreen::drawSessionStats(DisplayManager& display) {
    // Zeile 1: Clicks und Caps
    char line1[50];
    snprintf(line1, sizeof(line1), "Clicks: %d | Caps: %d",
             statsManager->getClicks(), statsManager->getCaps());
    display.drawText(10, 70, line1, TFT_WHITE, 2);

    // Zeile 2: Verbrauch
    String consumption = statsManager->getConsumption();
    char line2[40];
    snprintf(line2, sizeof(line2), "Verbrauch: %sg", consumption.c_str());
    display.drawText(10, 170, line2, TFT_WHITE, 2);
}

void FireScreen::update() {
    const bool isActive = heater.isHeating() || heater.getState() == HeaterController::State::COOLDOWN;

    // Update heating timer
    if (isActive) {
        state.lastActivityTime = millis();

        // Redraw timer jede Sekunde
        static uint32_t lastSecond = 0;
        const uint32_t currentSecond = (millis() - state.heatingStartTime) / 1000;
        if (currentSecond != lastSecond) {
            markDirty();
            lastSecond = currentSecond;
        }
    }

    // Check screensaver timeout
    checkScreensaverTimeout();
}

void FireScreen::handleInput(InputEvent event) {
    if (event.type != PRESS) return;

    resetActivityTimer();

    switch (event.button) {
        case FIRE:
            if (!heater.isHeating()) {
                heater.startHeating();
                state.heatingStartTime = millis();
            } else {
                const bool updateCycle = heater.isHeating();
                heater.stopHeating();
                if (updateCycle) {
                    setCycleCallback(state.currentCycle);
                    state.currentCycle = (state.currentCycle == 1) ? 2 : 1;
                }
            }
            markDirty();
            break;

        case UP:
            handleCycleChange(true);
            break;

        case DOWN:
            handleCycleChange(false);
            break;

        default:
            break;
    }
}

void FireScreen::handleCycleChange(bool increment) {
    if (increment) {
        state.currentCycle = (state.currentCycle % 4) + 1;
    } else {
        state.currentCycle = (state.currentCycle == 1) ? 4 : state.currentCycle - 1;
    }

    setCycleCallback(state.currentCycle);
    markDirty();
}

void FireScreen::checkScreensaverTimeout() {
    const bool isActive = heater.isHeating() || heater.getState() == HeaterController::State::COOLDOWN;

    if (!isActive && (millis() - state.lastActivityTime > SCREENSAVER_TIMEOUT_MS)) {
        screenManager->setScreen(screensaverScreen, ScreenTransition::FADE);
    }
}

void FireScreen::resetActivityTimer() {
    state.lastActivityTime = millis();
}
