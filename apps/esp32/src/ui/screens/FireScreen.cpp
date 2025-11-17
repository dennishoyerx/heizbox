#include "ui/screens/FireScreen.h"
#include "hardware/display/DisplayDriver.h"
#include "core/DeviceState.h"
#include "ui/base/ScreenManager.h"
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
                       ScreensaverScreen *ss, StatsManager *stm, TempSensor* ts)
    : heater(hc),
      screenManager(sm),
      screensaverScreen(ss),
      statsManager(stm),
      tempSensor(ts), // Initialize TempSensor
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

    state.targetTemp = DeviceState::instance().targetTemperature.get();
    state.power = DeviceState::instance().power.get();

    DeviceState::instance().targetTemperature.addListener([this](float val) { state.targetTemp = val; markDirty(); });
    DeviceState::instance().power.addListener([this](uint8_t val) { state.power = val; markDirty(); });
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
    _ui->withSurface(88, 50, 0, 60, {
        {"currentTemp", state.currentTemp}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);

        s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
        s.sprite->setFreeFont(&FreeSans18pt7b);

        // Current Temp
        s.sprite->drawBitmap(-10, 0, image_temp_48, 48, 48, COLOR_TEXT_PRIMARY);
        s.sprite->drawString(isnan(state.currentTemp) ? "Err" : String(state.currentTemp, 0), 30, 6);
    });

    _ui->withSurface(104, 50, 84, 60, {
        {"targetTemp", state.targetTemp}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
        s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
        s.sprite->setFreeFont(&FreeSans18pt7b);

        // Target Temp
        s.sprite->drawBitmap(0, 0, image_target_48, 48, 48, COLOR_TEXT_PRIMARY);
        s.sprite->drawString(String(state.targetTemp, 0), 46, 6);
    });

    _ui->withSurface(100, 40, 192, 60, {
        {"power", (int)heater.getPower()}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);

        // Power
        s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
        s.sprite->setFreeFont(&FreeSans18pt7b);
        s.sprite->drawString(String(heater.getPower()), 32, 6);
        s.sprite->drawBitmap(-10, 0, image_power_48, 48, 48, COLOR_TEXT_PRIMARY);
    });


    const bool isHeating = heater.isHeating();

    _ui->withSurface(250, 140, 15, 115, {
        {"isHeating", isHeating},
        {"consumption", cachedConsumption},
        {"todayConsumption", cachedTodayConsumption},
        {"currentCycle", state.currentCycle}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
        FireScreen::drawSessionRow(s.sprite, "Session", cachedConsumption, 0, COLOR_BG_2, COLOR_BG_2, COLOR_TEXT_PRIMARY, (state.currentCycle == 1));
        FireScreen::drawSessionRow(s.sprite, "Heute", cachedTodayConsumption, 50, COLOR_BG_3, COLOR_BG_2, COLOR_TEXT_PRIMARY);
    });


    if (!isHeating) {
        return;
    }

    const uint32_t seconds = (heater.getElapsedTime()) / 1000;
    _ui->withSurface(140, 140, 70, 100, {
        {"seconds", (int)seconds}
    }, [this, seconds](RenderSurface& s) {
        drawHeatingTimer(s.sprite, seconds);
    });
}

void FireScreen::drawHeatingTimer(TFT_eSprite* sprite, uint32_t seconds)
{
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
}

void FireScreen::update()
{
    const bool isActive = heater.isHeating() || heater.isPaused();

    tempSensor->update();
    float temp = tempSensor->getTemperature();
    
    if (temp != state.currentTemp) {
        state.currentTemp = temp;
        markDirty();
    }

    if (isActive)
    {
        static uint32_t lastSecond = 0;
        const uint32_t currentSecond = heater.getElapsedTime() / 1000;
        if (currentSecond != lastSecond)
        {
            markDirty();
            lastSecond = currentSecond;
        }

        if (state.currentTemp > state.targetTemp) {
            _handleHeatingTrigger(false);
            markDirty();
        }
    }

    static bool wasHeating = false;
    if (!heater.isHeating() && wasHeating) {
        _ui->clear();
        markDirty();
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

    if (event.button == CENTER) {
        handleCycleChange();
        return;
    }

    if (event.button == UP) {
        DeviceState::instance().targetTemperature.set(DeviceState::instance().targetTemperature.get() + 1);
        return;
    }

    if (event.button == DOWN) {
        DeviceState::instance().targetTemperature.set(DeviceState::instance().targetTemperature.get() - 1);
        return;
    }

    if (event.button == LEFT) {
        uint8_t _power = DeviceState::instance().power.get() - 10;
        if (_power < 10) {
            _power = 10;
        }

        DeviceState::instance().power.set(_power);
        markDirty();
        return;
    }

    if (event.button == RIGHT) {
        uint8_t _power = DeviceState::instance().power.get() + 10;
       if (_power > 100) {
            _power = 100;
        }

        DeviceState::instance().power.set(_power);
        markDirty();
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
    /*if (!isActive && (millis() - state.lastActivityTime > DeviceState::instance().sleepTimeout.get()))
    {
        screenManager->setScreen(screensaverScreen, ScreenTransition::FADE);
    }*/
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
