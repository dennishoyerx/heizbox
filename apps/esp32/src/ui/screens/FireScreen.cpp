#include "ui/screens/FireScreen.h"
#include "hardware/display/DisplayDriver.h"
#include "core/Config.h"
#include "core/DeviceState.h"
#include "ui/base/ScreenManager.h"
#include "ui/ColorPalette.h"
#include "bitmaps.h"
#include <TFT_eSPI.h>
#include "utils/Logger.h"
#include "StateManager.h"
#include <utility>

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

FireScreen::FireScreen(HeaterController &hc) : heater(hc) {
    auto& ds = DeviceState::instance();

    bindTo(state.consumption, ds.sessionConsumption);
    bindTo(state.todayConsumption, ds.todayConsumption);
    bindTo(state.yesterdayConsumption, ds.yesterdayConsumption);
    bindTo(state.targetTemp, ds.targetTemperature);
    bindTo(state.power, ds.power);
    bindTo(state.currentCycle, ds.currentCycle);
}

void FireScreen::draw(DisplayDriver &display)
{
    // Current Temp
    _ui->withSurface(88, 50, 0, 60, {
        {"currentTemp", state.currentTemp}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);

        s.sprite->drawBitmap(-10, 0, image_temp_48, 48, 48, COLOR_TEXT_PRIMARY);
        s.text(30, 6, isnan(state.currentTemp) ? "Err" : String(state.currentTemp, 0), TextSize::lg);
    });

    // Target Temp
    _ui->withSurface(104, 50, 84, 60, {
        {"targetTemp", state.targetTemp}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);

        s.sprite->drawBitmap(0, 0, image_target_48, 48, 48, COLOR_TEXT_PRIMARY);
        s.text(40, 6, String(state.targetTemp, 0), TextSize::lg);
    });

    // Power
    _ui->withSurface(100, 40, 192, 60, {
        {"power", state.power}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
        s.text(30, 6, String(state.power), TextSize::lg);
        s.sprite->drawBitmap(-10, 0, image_power_48, 48, 48, COLOR_TEXT_PRIMARY);
    });


    const bool isHeating = heater.isHeating();

    // Consumption
    _ui->withSurface(250, 140, 15, 115, {
        {"isHeating", isHeating},
        {"consumption", state.consumption},
        {"todayConsumption", state.todayConsumption},
        {"currentCycle", state.currentCycle}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
        FireScreen::drawSessionRow(s.sprite, "Session", state.consumption, 0, COLOR_BG_2, COLOR_BG_2, COLOR_TEXT_PRIMARY, (state.currentCycle == 1));
        FireScreen::drawSessionRow(s.sprite, "Heute", state.todayConsumption, 50, COLOR_BG_3, COLOR_BG_2, COLOR_TEXT_PRIMARY);
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

    float progress = min(progress, 1.0f);
    int endAngle = (int)(progress * 360);
    int startAngle = 180; 
    int stopAngle = startAngle + endAngle;

    sprite->drawArc(centerX, centerY,
                    radius + 7, radius - 7,
                    startAngle, stopAngle,
                    timerColor, COLOR_ACCENT, true);

    // === TIMER ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", seconds);
    
    sprite->setTextColor(COLOR_TEXT_PRIMARY);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(2);
    sprite->setFreeFont(&FreeSansBold18pt7b);
    sprite->drawString(timeStr, centerX, centerY, 1);
}

void FireScreen::update() {
    const bool isActive = heater.isHeating() || heater.isPaused();
    float temp = heater.getTemperature();
    
    if (temp != state.currentTemp) {
        state.currentTemp = temp;
        markDirty();
    }

    if (isActive) {
        if (state.currentTemp > state.targetTemp) {
            _handleHeatingTrigger(false);
            markDirty();
        }

        static uint32_t lastSecond = 0;
        state.elapsedSeconds = heater.getElapsedTime() / 1000;
        if (state.elapsedSeconds != lastSecond) {
            lastSecond = state.elapsedSeconds;
            markDirty();
        }

    }

    static bool wasHeating = false;
    if (!heater.isHeating() && wasHeating) {
        _ui->clear();
        markDirty();
    }
    wasHeating = heater.isHeating();
}

void FireScreen::handleInput(InputEvent event) {
    if (event.button == UP || event.button == DOWN && 
        event.type == PRESS || event.type == HOLD || event.type == HOLDING) {
        float targetTempUpdate = event.button == UP ? 1 : -1;
        DeviceState::instance().targetTemperature.update([targetTempUpdate](float val) { return val + targetTempUpdate; });
        return;
    }

    bool triggerHeating = (event.button == FIRE) || (event.button == CENTER && DeviceState::instance().enableCenterButtonForHeating.get());

    if (triggerHeating) {
        _handleHeatingTrigger(!heater.isHeating());
        return;
    }

    if (event.button == CENTER) {
        DeviceState::instance().currentCycle.update([](uint8_t val) { return val == 1 ? 2 : 1; });
        return;
    }

    if (event.button == LEFT || event.button == RIGHT) {
        uint8_t powerUpdate = event.button == LEFT ? -10 : 10;
        DeviceState::instance().power.update([powerUpdate](uint8_t val) { 
            uint8_t newVal = val + powerUpdate;
            if (newVal < 30) newVal = 30;
            if (newVal > 100) newVal = 100;
            return newVal; 
        });
        return;
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
