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

        uint8_t bgColor = highlight ? COLOR_TEXT_PRIMARY : COLOR_ACCENT;
        uint8_t textColor = highlight ? COLOR_ACCENT : COLOR_TEXT_PRIMARY;
        uint8_t shadowColor = highlight ? COLOR_BORDER : COLOR_TEXT_PRIMARY;

        display.fillRect(x, y, width, height, shadowColor);
        display.fillRect(x + 2, y + 2, width - 4, height - 4, bgColor);

        int iconX = x + 12;
        int iconY = y + 15;
        int iconSize = 20;
        display.drawRect(iconX, iconY, iconSize, iconSize, textColor);

        display.drawText(iconX + iconSize + 10, y + 32, label, textColor, 2);

        char consumptionStr[10];
        snprintf(consumptionStr, sizeof(consumptionStr), "%.2fg", consumption);
        int textWidth = display.getTextWidth(consumptionStr, 3);
        display.drawText(x + width - 12 - textWidth, y + 35, consumptionStr, textColor, 3);
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

    if (heater.isHeating() || heater.isPaused()) {
        drawHeatingTimer(display);
    } 
    
    drawSessionRow(display, "Session", cachedConsumption, 10, (state.currentCycle == 1));
    drawSessionRow(display, "Heute", cachedTodayConsumption, 65, false);
    drawSessionRow(display, "Gestern", cachedYesterdayConsumption, 120, false);
  
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

    uint8_t timerColorIndex;
    if (seconds < 20) timerColorIndex = COLOR_SUCCESS;
    else if (seconds < 35) timerColorIndex = COLOR_WARNING;
    else if (seconds < 50) timerColorIndex = COLOR_ACCENT;
    else timerColorIndex = COLOR_ERROR;
    
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
    int startAngle = 180;              // 12 Uhr bei Rotation 1
    int stopAngle = startAngle + endAngle;

    renderer.drawArc(centerX, centerY,
                    radius + 3, radius - 3,
                    startAngle, stopAngle,
                    timerColor, 0xFB40, true);

    // === Große Zahl ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", seconds % 60);
    
    renderer.setTextColor(COLOR_TEXT_PRIMARY);
    renderer.setTextDatum(MC_DATUM);
    renderer.setTextSize(2);
    renderer.setFreeFont(&FreeSansBold18pt7b);
    renderer.drawString(timeStr, centerX, centerY, 1);
    
    
    // "HEIZT" or "PAUSE" Badge
    const char* badgeText = heater.isPaused() ? "PAUSE" : "HEIZT";
    uint16_t badgeColor = heater.isPaused() ? COLOR_WARNING : timerColor;
    renderer.fillRoundRect(centerX - 35, centerY + 50, 70, 20, 10, 0x8410);
    renderer.fillCircle(centerX - 20, centerY + 60, 3, badgeColor);
    renderer.setFreeFont(&FreeSans18pt7b);
    renderer.setTextSize(1);
    renderer.drawString(badgeText, centerX + 5, centerY + 60, 2);
    
    // Click Zone
    if (seconds >= 30 && seconds <= 50) {
    renderer.setFreeFont(&FreeSans18pt7b);
        renderer.setTextSize(1);
        renderer.setTextColor(COLOR_BG_DARK);
        renderer.drawString("CLICK ZONE", centerX, centerY + 80, 2);
    }
}

void FireScreen::drawStatus(DisplayDriver &display)
{
    const char *status = nullptr;
    switch (heater.getState())
    {
    case HeaterController::State::COOLDOWN: status = "COOLDOWN"; break;
    case HeaterController::State::ERROR: status = "ERROR"; break;
    default: break;
    }

    if (status)
    {
        int textWidth = display.getTextWidth(status, 3);
        display.drawText(140 - textWidth/2, 115, status, COLOR_TEXT_SECONDARY, 3);
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
