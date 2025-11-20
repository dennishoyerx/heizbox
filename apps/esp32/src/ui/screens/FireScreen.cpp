#include "ui/screens/FireScreen.h"
#include "ui/components/HeatUI.h"
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
        s.text(30, 6, isnan(state.currentTemp) ? "Err" : String(state.currentTemp), TextSize::lg);
    });

    // Target Temp
    _ui->withSurface(104, 50, 84, 60, {
        {"targetTemp", state.targetTemp}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);

        s.sprite->drawBitmap(0, 0, image_target_48, 48, 48, COLOR_TEXT_PRIMARY);
        s.text(40, 6, String(state.targetTemp), TextSize::lg);
    });

    // Power
    _ui->withSurface(100, 40, 192, 60, {
        {"power", state.power}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
        s.text(30, 6, String(state.power), TextSize::lg);
        s.sprite->drawBitmap(-10, 0, image_power_48, 48, 48, COLOR_TEXT_PRIMARY);
    });

    // Consumption
    _ui->withSurface(250, 140, 15, 115, {
        {"isHeating", state.isHeating},
        {"consumption", state.consumption},
        {"todayConsumption", state.todayConsumption},
        {"currentCycle", state.currentCycle}
    }, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
        FireScreen::drawSessionRow(s.sprite, "Session", state.consumption, 0, COLOR_BG_2, COLOR_BG_2, COLOR_TEXT_PRIMARY, (state.currentCycle == 1));
        FireScreen::drawSessionRow(s.sprite, "Heute", state.todayConsumption, 50, COLOR_BG_3, COLOR_BG_2, COLOR_TEXT_PRIMARY);
    });


    if (!state.isHeating) {
        return;
    }

    float progress = min(progress, 1.0f);
    _ui->withSurface(280, 140, 0, 100, {
        {"seconds", (int)state.elapsedSeconds},
        {"progress", state.progress},
    }, [this](RenderSurface& s) {
        HeatUI(s, state.elapsedSeconds, state.progress);
    });
}

void FireScreen::update() {
    state.isHeating = heater.isHeating();
    uint16_t temp = heater.getTemperature();
    
    if (temp != state.currentTemp) {
        state.currentTemp = temp;
        markDirty();
    }

    if (state.isHeating) {
        
        state.progress = (float)state.currentTemp / state.targetTemp;
        if (state.progress > 1.0f) state.progress = 1.0f;

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
    if (!state.isHeating && wasHeating) {
        _ui->clear();
        markDirty();
    }
    wasHeating = state.isHeating;
}

void FireScreen::handleInput(InputEvent event) {
    if (event.button == UP || event.button == DOWN && 
        event.type == PRESS || event.type == HOLD || event.type == HOLDING) {
        float targetTempUpdate = event.button == UP ? 1 : -1;
        DeviceState::instance().targetTemperature.update([targetTempUpdate](uint8_t val) { return val + targetTempUpdate; });
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
