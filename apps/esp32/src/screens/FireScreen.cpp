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
    centerText(display, 100, timeStr, TFT_WHITE, 5);
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
    snprintf(text, sizeof(text), "%d", state.currentCycle);
    display.drawBitmap(10, 116-30, epd_bitmap_fire, 32, 32, TFT_WHITE);
    display.drawText(52, 116, text, TFT_WHITE, 4);
}

void FireScreen::drawSessionStats(DisplayManager& display) {
    // Zeile 1: Clicks und Caps
    char lineCaps[50];
    snprintf(lineCaps, sizeof(lineCaps), "%d Caps",
             statsManager->getCaps());
    char lineClicks[50];
    snprintf(lineClicks, sizeof(lineClicks), "%d Clicks",
             statsManager->getClicks());

    // Zeile 2: Verbrauch
    String consumption = statsManager->getConsumption();
    char lineConsumption[40];
    snprintf(lineConsumption, sizeof(lineConsumption), "%sg", consumption.c_str());

    display.drawBitmap(10, 150 - 16, image_session, 32, 32, TFT_WHITE);
    display.drawText(152, 155, lineCaps, TFT_WHITE, 2);
    display.drawText(152, 180, lineClicks, TFT_WHITE, 2);
    display.drawText(52, 162, lineConsumption, TFT_WHITE, 3);
    //display.drawText(52, 150, line1, TFT_WHITE, 2);
    //display.drawText(52, 175, line2, TFT_WHITE, 2);
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
                if (updateCycle && (millis() - state.heatingStartTime > 10000)) {
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

    if (!isActive && (millis() - state.lastActivityTime > Config::Timing::SCREENSAVER_TIMEOUT_MS)) {
        screenManager->setScreen(screensaverScreen, ScreenTransition::FADE);
    }
}

void FireScreen::resetActivityTimer() {
    state.lastActivityTime = millis();
}
