#include "ui/screens/FireScreen.h"
#include "hardware/display/DisplayDriver.h"
#include "core/DeviceState.h"
#include "ui/base/ScreenManager.h"
#include "ui/components/UIText.h"
#include "ui/ColorPalette.h"
#include "bitmaps.h"
#include <TFT_eSPI.h>
#include "utils/Logger.h"
#include "StateManager.h"
#include <utility>

//#enum SessionRowStyle {};

void FireScreen::drawSessionRow(TFT_eSprite* sprite, 
    const char* label, 
    float consumption, 
    int y, 
    uint8_t bgColor, 
    uint8_t borderColor, 
    uint8_t textColor, 
    bool invert, 
    bool thin)
{    
    int x = 0;
    int width = 250; 
    int height = thin ? 40 : 50;
    int radius = 16;
    uint8_t _bgColor;
    uint8_t _textColor;

    if (!invert) {
        _bgColor = bgColor;
        _textColor = textColor;
    } else {
        _bgColor = textColor;
        _textColor = bgColor;
    }

        sprite->fillSmoothRoundRect(x, y, width, height, radius, _bgColor, _textColor);

        // "Session" Text
        sprite->setTextSize(1);
        sprite->setTextColor(_textColor);
        sprite->setTextDatum(ML_DATUM);
        sprite->setFreeFont(thin ? &FreeSans9pt7b : &FreeSans12pt7b);
        sprite->drawString(label, 30, y + height / 2);

        // Verbrauchswert formatieren und anzeigen
        char consumptionStr[10];
        int integer = (int)consumption;
        int decimal = ((int)(consumption * 100 + 0.5f)) % 100;
        if (integer > 0)
        {
            sprintf(consumptionStr, "%d.%02dg", integer, decimal);
        }
        else
        {
            sprintf(consumptionStr, ".%02dg", decimal);
        }

        // Verbrauchswert rechts ausgerichtet
        sprite->setTextDatum(MR_DATUM);
        sprite->setFreeFont(thin ? &FreeSans9pt7b : &FreeSans18pt7b);
        sprite->drawString(consumptionStr, x + width - 12, y + height / 2);
}



 
FireScreen::FireScreen(HeaterController &hc, ScreenManager *sm,
                       ScreensaverScreen *ss, StatsManager *stm)
    : heater(hc),
      screenManager(sm),
      screensaverScreen(ss),
      statsManager(stm),
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
        DeviceState::instance().currentCycle.set(state.currentCycle);
        state.currentCycle = (state.currentCycle == 1) ? 2 : 1;
    }

    markDirty();
}

void FireScreen::draw(DisplayDriver &display)
{
    display.clear();

    _ui->withSurface(250, 140, 15, 75, [this](RenderSurface& s) {
        //TimerState st{ 3900, true }; // 65min
        s.sprite->fillSprite(COLOR_BG);
        FireScreen::drawSessionRow(s.sprite, "Session", cachedConsumption, 0, COLOR_BG_2, COLOR_BG_2, COLOR_TEXT_PRIMARY, (state.currentCycle == 1));
        FireScreen::drawSessionRow(s.sprite, "Heute", cachedTodayConsumption, 50, COLOR_BG_3, COLOR_BG_2, COLOR_TEXT_PRIMARY);
    });


    if (heater.isHeating()) {
    _ui->withSurface(140, 140, 70, 75, [this](RenderSurface& s) {
        //TimerState st{ 3900, true }; // 65min
        //s.sprite->fillSprite(COLOR_BG);
        drawHeatingTimer(s.sprite);
    });
    }
}

void FireScreen::drawHeatingTimer(TFT_eSprite* sprite)
{
    const uint32_t elapsed = heater.getElapsedTime();
    const uint32_t seconds = elapsed / 1000;
    
    static uint32_t lastSeconds = 999;
    if (seconds == lastSeconds && heater.isHeating()) { // Only skip redraw if actively heating and second hasn't changed
        //sprite->deleteSprite();
        return;
    }
    lastSeconds = seconds;

    uint8_t timerColor;
    if (seconds < 20) timerColor = COLOR_SUCCESS;
    else if (seconds < 35) timerColor = COLOR_WARNING;
    else if (seconds < 50) timerColor = COLOR_BLUE;
    else timerColor = COLOR_PURPLE;
    
    int centerX = 70;
    int centerY = 70;
    
    // === Vereinfachter Progress Ring ===
    int radius = 70;

    // Hintergrund-Ring
    sprite->fillCircle(centerX, centerY, radius, COLOR_BG);
    sprite->drawCircle(centerX, centerY, radius + 4, COLOR_TEXT_PRIMARY);

    float progress = (float)seconds / 60.0f;
    int endAngle = (int)(progress * 360);
    int startAngle = 180; 
    int stopAngle = startAngle + endAngle;

    sprite->drawArc(centerX, centerY,
                    radius + 7, radius - 7,
                    startAngle, stopAngle,
                    timerColor, COLOR_ACCENT, true);

    // === TIMER ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", seconds % 60);
    
    sprite->setTextColor(COLOR_TEXT_PRIMARY);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(2);
    sprite->setFreeFont(&FreeSansBold18pt7b);
    sprite->drawString(timeStr, centerX, centerY, 1);
    
    
    // "HEIZT" or "PAUSE" Badge
    const char* badgeText = heater.isPaused() ? "PAUSE" : "HEIZT";
    uint16_t badgeColor = heater.isPaused() ? COLOR_WARNING : timerColor;
    sprite->fillRoundRect(centerX - 35, centerY + 50, 70, 20, 10, 0x8410);
    sprite->fillCircle(centerX - 20, centerY + 60, 3, badgeColor);
    sprite->setFreeFont(&FreeSans18pt7b);
    sprite->setTextSize(1);
    sprite->drawString(badgeText, centerX + 5, centerY + 60, 2);
    
    // Click Zone
    if (seconds >= 30 && seconds <= 50) {
    sprite->setFreeFont(&FreeSans18pt7b);
        sprite->setTextSize(1);
        sprite->setTextColor(COLOR_BG);
        sprite->drawString("CLICK ZONE", centerX, centerY + 80, 2);
    }

//    sprite->pushSprite(70, 75);
//    sprite->deleteSprite();
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
    DeviceState::instance().currentCycle.set(state.currentCycle);
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
