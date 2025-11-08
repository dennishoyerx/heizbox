#include "ui/screens/FireScreen.h"
#include "core/DeviceState.h"
#include "ui/base/ScreenManager.h"
#include "ui/components/UIText.h"
#include "ui/ColorPalette.h"
#include "bitmaps.h"
#include <TFT_eSPI.h>
#include "utils/Logger.h"
#include "StateManager.h"
#include <utility>

namespace {
    void drawSessionRow(DisplayDriver &display, const char* label, float consumption, int y, bool highlight = false)
    {    
        int x = 10;
        int width = 260; 
        int height = 50;
        int radius = 8;
        uint8_t bgColor = highlight ? CustomColors::COLOR_TEXT_PRIMARY : heizbox_palette[CustomColors::COLOR_ACCENT];
        uint8_t textColor = highlight ? heizbox_palette[CustomColors::COLOR_ACCENT] : CustomColors::COLOR_TEXT_PRIMARY;
        uint8_t shadowColor = highlight ? heizbox_palette[CustomColors::COLOR_BORDER] : CustomColors::COLOR_TEXT_PRIMARY;


        auto &renderer = display.getRenderer();

        if (highlight)
        {
            renderer.fillSmoothRoundRect(x, y, width, height, radius, bgColor, textColor);
        }
        else
        {
            renderer.fillSmoothRoundRect(x, y, width, height, radius, shadowColor, shadowColor);
            renderer.fillSmoothRoundRect(x + 2, y + 2, width - 4, height - 4, radius, bgColor, bgColor);
        }
        
        // "Session" Text
        renderer.setTextSize(1);
        renderer.setTextColor(textColor);
        renderer.setTextDatum(ML_DATUM);
        renderer.setFreeFont(&FreeSans12pt7b);
        renderer.drawString(label, 10, y + height / 2);

        // Verbrauchswert formatieren und anzeigen
        char consumptionStr[10];
        int integer = (int)consumption;
        int decimal = (int)(consumption * 100) % 100;
        if (integer > 0)
        {
            sprintf(consumptionStr, "%d.%02dg", integer, decimal);
        }
        else
        {
            sprintf(consumptionStr, ".%02dg", decimal);
        }

        // Verbrauchswert rechts ausgerichtet
        renderer.setTextDatum(MR_DATUM);
        renderer.setFreeFont(&FreeSansBold18pt7b);
        renderer.drawString(consumptionStr, x + width - 12, y + height / 2);
    }
}

FireScreen::FireScreen(HeaterController &hc, ScreenManager *sm,
                       ScreensaverScreen *ss, StatsManager *stm,
                       std::function<void(int)> setCycleCb)
    : heater(hc),
      screenManager(sm),
      screensaverScreen(ss),
      statsManager(stm),
      setCycleCallback(std::move(setCycleCb)),
      cachedClicks(0),
      cachedConsumption(0),
      cachedTodayConsumption(0),
      cachedYesterdayConsumption(0),
      cachedCaps(0)
{
    DeviceState::instance().sessionClicks.addListener([this](int val) { cachedClicks = val; markDirty(); });
    DeviceState::instance().sessionCaps.addListener([this](int val) { cachedCaps = val; markDirty(); });
    DeviceState::instance().sessionConsumption.addListener([this](double val) { cachedConsumption = val; markDirty(); });
    DeviceState::instance().todayConsumption.addListener([this](double val) { cachedTodayConsumption = val; markDirty(); });
    DeviceState::instance().yesterdayConsumption.addListener([this](double val) { cachedYesterdayConsumption = val; markDirty(); });

    state.lastActivityTime = millis();
    state.currentCycle = 1;
    state.showingSavedConfirmation = false;
    state.confirmationStartTime = 0;
}

void FireScreen::onEnter()
{
    resetActivityTimer();
}

void FireScreen::onCycleFinalized() {
    if (heater.getLastCycleDuration() > 10000) {
        setCycleCallback(state.currentCycle);
        state.currentCycle = (state.currentCycle == 1) ? 2 : 1;
    }
}

void FireScreen::draw(DisplayDriver &display)
{
    display.clear();
    
    drawSessionRow(display, "Session", cachedConsumption, 10, (state.currentCycle == 1));
    drawSessionRow(display, "Heute", cachedTodayConsumption, 65, false);
    drawSessionRow(display, "Gestern", cachedYesterdayConsumption, 120, false);

    if (heater.isHeating()) {
        drawHeatingTimer(display);
    }
}

void FireScreen::drawHeatingTimer(DisplayDriver &display)
{
    auto &renderer = display.getRenderer();
    
    const uint32_t elapsed = heater.getElapsedTime();
    const uint32_t seconds = elapsed / 1000;
    
    static uint32_t lastSeconds = 999;
    if (seconds == lastSeconds && heater.isHeating()) { // Only skip redraw if actively heating and second hasn't changed
        return;
    }
    lastSeconds = seconds;

    uint8_t timerColor;
    if (seconds < 20) timerColor = heizbox_palette[CustomColors::COLOR_SUCCESS];
    else if (seconds < 35) timerColor = heizbox_palette[CustomColors::COLOR_WARNING];
    else if (seconds < 50) timerColor = heizbox_palette[CustomColors::COLOR_ACCENT];
    else timerColor = heizbox_palette[CustomColors::COLOR_ERROR];
    
    int centerX = 140;
    int centerY = 96;
    
    // === Vereinfachter Progress Ring ===
    int radius = 70;

    renderer.fillCircle(centerX, centerY, radius, 0xFB40);
    // Hintergrund-Ring
    renderer.drawCircle(centerX, centerY, radius, 0x8410);
    renderer.drawCircle(centerX, centerY, radius-1, 0x8410);
    renderer.drawCircle(centerX, centerY, radius-2, 0x8410);

    float progress = (float)seconds / 60.0f;
    int endAngle = (int)(progress * 360);
    int startAngle = 180; 
    int stopAngle = startAngle + endAngle;

    renderer.drawArc(centerX, centerY,
                    radius + 3, radius - 3,
                    startAngle, stopAngle,
                    timerColor, heizbox_palette[CustomColors::COLOR_ACCENT], true);

    // === TIMER ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", seconds % 60);
    
    renderer.setTextColor(heizbox_palette[CustomColors::COLOR_TEXT_PRIMARY]);
    renderer.setTextDatum(MC_DATUM);
    renderer.setTextSize(2);
    renderer.setFreeFont(&FreeSansBold18pt7b);
    renderer.drawString(timeStr, centerX, centerY, 1);
    
    
    // "HEIZT" or "PAUSE" Badge
    const char* badgeText = heater.isPaused() ? "PAUSE" : "HEIZT";
    uint16_t badgeColor = heater.isPaused() ? heizbox_palette[CustomColors::COLOR_WARNING] : timerColor;
    renderer.fillRoundRect(centerX - 35, centerY + 50, 70, 20, 10, 0x8410);
    renderer.fillCircle(centerX - 20, centerY + 60, 3, badgeColor);
    renderer.setFreeFont(&FreeSans18pt7b);
    renderer.setTextSize(1);
    renderer.drawString(badgeText, centerX + 5, centerY + 60, 2);
    
    // Click Zone
    if (seconds >= 30 && seconds <= 50) {
    renderer.setFreeFont(&FreeSans18pt7b);
        renderer.setTextSize(1);
        renderer.setTextColor(heizbox_palette[CustomColors::COLOR_BG_DARK]);
        renderer.drawString("CLICK ZONE", centerX, centerY + 80, 2);
    }
}

void FireScreen::update()
{
    const bool isActive = heater.isHeating() || heater.isPaused() || heater.getState() == HeaterController::State::COOLDOWN;

    if (isActive)
    {
        state.lastActivityTime = millis();
        static uint32_t lastSecond = 0;
        const uint32_t currentSecond = heater.getElapsedTime() / 1000;
        if (currentSecond != lastSecond)
        {
            markDirty();
            lastSecond = currentSecond;
        }
    }

    static bool wasHeating = false;
    if (!heater.isHeating() && wasHeating) {
        markDirty(); // Force redraw when heating stops
    }
    wasHeating = heater.isHeating();

    checkScreensaverTimeout();
}

void FireScreen::handleInput(InputEvent event)
{
    if (event.type != PRESS) return;

    resetActivityTimer();

    bool triggerHeating = (event.button == FIRE) || (event.button == CENTER && DeviceState::instance().enableCenterButtonForHeating.get());

    if (triggerHeating)
    {
        _handleHeatingTrigger(!heater.isHeating());
        return;
    }

    if (event.button == UP || event.button == DOWN) {
        handleCycleChange();
    }
}

void FireScreen::handleCycleChange()
{
    state.currentCycle = (state.currentCycle == 1) ? 2 : 1;
    setCycleCallback(state.currentCycle);
    markDirty();
}

void FireScreen::checkScreensaverTimeout()
{
    const bool isActive = heater.isHeating() || heater.isPaused() || heater.getState() == HeaterController::State::COOLDOWN;
    if (!isActive && (millis() - state.lastActivityTime > DeviceState::instance().sleepTimeout.get()))
    {
        screenManager->setScreen(screensaverScreen, ScreenTransition::FADE);
    }
}

void FireScreen::_handleHeatingTrigger(bool shouldStartHeating)
{
    if (shouldStartHeating) {
        heater.startHeating();
    } else if (heater.isHeating()) {
        heater.stopHeating(false);
    }
    markDirty();
}

void FireScreen::resetActivityTimer()
{
    state.lastActivityTime = millis();
}
