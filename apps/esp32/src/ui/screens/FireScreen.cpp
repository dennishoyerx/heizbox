#include "ui/screens/FireScreen.h"
#include "ui/components/MenuBuilder.h"
#include "ui/components/HeatUI.h"
#include "DisplayDriver.h"
#include "Config.h"
#include "core/DeviceState.h"
#include "ui/ColorPalette.h"
#include "bitmaps.h"
#include "heater\HeaterCycle.h"
#include "core/EventBus.h"
 
#include <utility>

#include "utils/Logger.h"

FireScreen::FireScreen(HeaterController &hc) : heater(hc) {
    auto& ds = DeviceState::instance();

    bindTo(state.consumption.session, ds.sessionConsumption);
    bindTo(state.consumption.today, ds.todayConsumption);
    bindTo(state.consumption.yesterday, ds.yesterdayConsumption);
    bindTo(state.heater.targetTemp, ds.targetTemperature);
    bindTo(state.heater.power, ds.power);
    bindTo(state.heater.currentCycle, ds.currentCycle);
    bindTo(state.heater.isHeating, ds.isHeating);

    ds.currentCycle.addListener([this](uint8_t val) {
        auto& ds = DeviceState::instance();

        if (val == 1) {
            state.heater.targetTemp = ds.targetTemperature.set(ds.targetTemperatureCycle1.get());
            dirty();
        } else if (val == 2) {
            state.heater.targetTemp = ds.targetTemperature.set(ds.targetTemperatureCycle2.get());
        }
        dirty();
    });
}

void FireScreen::draw() {
        ZVSDriver* zvs = heater.getZVSDriver();
    if (state.heater.isHeating) {
        HeatUI(_ui, state.heater, zvs);

        return;
    }

    
    // Consumption
    _ui->withSurface(250, 140, 15, 110, {
        {"isHeating", state.heater.isHeating},
        {"consumption", state.consumption.session},
        {"todayConsumption", state.consumption.today},
        {"currentCycle", state.heater.currentCycle}
    }, [this](RenderSurface& s) {
        FireScreen::drawSessionRow(s.sprite, "Session", state.consumption.session, 0, COLOR_BG_2, COLOR_BG_2, COLOR_TEXT_PRIMARY, (state.heater.currentCycle == 1));
        FireScreen::drawSessionRow(s.sprite, "Heute", state.consumption.today, 55, COLOR_BG_3, COLOR_BG_2, COLOR_TEXT_PRIMARY);
        FireScreen::drawSessionRow(s.sprite, "Gestern", state.consumption.yesterday, 105, COLOR_BG, COLOR_BG, COLOR_TEXT_PRIMARY, false, true);
    });

    // Current Temp
    _ui->withSurface(88, 50, 0, 45, {
        {"currentTemp", state.heater.currentTemp}
    }, [this](RenderSurface& s) {
        s.sprite->drawBitmap(-5, 0, image_temp_40, 40, 40, COLOR_TEXT_PRIMARY);
        s.text(30, 6, isnan(state.heater.currentTemp) ? "Err" : String(state.heater.currentTemp), TextSize::lg);
    });

    // Target Temp
    _ui->withSurface(104, 50, 84, 45, {
        {"targetTemp", state.heater.targetTemp}
    }, [this](RenderSurface& s) {
        s.sprite->drawBitmap(0, 0, image_target_40, 40, 40, COLOR_TEXT_PRIMARY);
        s.text(40, 6, String(state.heater.targetTemp), TextSize::lg);
    });

    // Power
    _ui->withSurface(100, 40, 192, 45, {
        {"power", state.heater.power}
    }, [this](RenderSurface& s) {
        s.sprite->drawBitmap(-10, 0, image_power_40, 40, 40, COLOR_TEXT_PRIMARY);
        s.text(30, 6, String(state.heater.power), TextSize::lg);
    });

    // Seperator
    _ui->withSurface(280, 1, 0, 95, [this](RenderSurface& s) {
        s.sprite->drawRect(0, 0, 280, 1, COLOR_BG_2);
    });

}

void FireScreen::update() {
    uint16_t temp = heater.getTemperature();
    
    if (temp != state.heater.currentTemp) {
        state.heater.currentTemp = temp;
        dirty();
    }

    if (state.heater.isHeating) {
        state.heater.elapsedSeconds = heater.getElapsedTime() / 1000;
        state.heater.progress = (float)state.heater.currentTemp / state.heater.targetTemp;

        if (state.heater.progress > 1.0f) state.heater.progress = 1.0f;

        if (state.heater.currentTemp > state.heater.targetTemp) {
            _handleHeatingTrigger(false);
            dirty();
        }

        static uint32_t lastSecond = 0;
        if (state.heater.elapsedSeconds != lastSecond) {
            lastSecond = state.heater.elapsedSeconds;
            dirty();
        }
        dirty();
    }

    static bool wasHeating = false;
    if (!state.heater.isHeating && wasHeating) {
        _ui->clear();
        dirty();
    }
    wasHeating = state.heater.isHeating;
}

std::vector<std::unique_ptr<MenuItem>>  FireScreen::buildMenu() {
    auto& state = DeviceState::instance();

    return MenuBuilder()
         .addHeadline("ZVS ADVANCED")
            .addObservableRangeMs("Duty Period", state.zvsDutyCyclePeriodMs, 200, 2000, 100)
            .addObservableRangeMs("Temp Sensor", state.tempSensorOffTime, 50, 220, 10)
         
         .build();
}

bool triggeredTwice(uint32_t intervalMs) {
    static uint32_t lastTime = 0;
    uint32_t now = millis();
    if (now - lastTime <= intervalMs) {
        lastTime = 0;
        return true;
    }
    lastTime = now;
    return false;
}

void FireScreen::handleInput(InputEvent event) {
    auto& ds = DeviceState::instance();
    if ((event.button == UP || event.button == DOWN) && 
        (event.type == PRESS || event.type == HOLD)) {
        float delta = event.button == UP ? 1 : -1;
        uint8_t temp;

        if (HeaterCycle::currentCycle() == 1) {
            temp = ds.targetTemperatureCycle1.update([delta](uint8_t val) { return val + delta; });
        } else {
            temp = ds.targetTemperatureCycle2.update([delta](uint8_t val) { return val + delta; });
            //ds.heatCycleTempDelta.update([delta](uint8_t val) { return val + delta; });
            //state.heater.targetTemp = ds.targetTemperature + ds.heatCycleTempDelta;
        }
        ds.targetTemperature.set(temp);
        dirty();
        
        return;
    }


    static uint32_t _temp;
    if (event.button == FIRE && event.type == PRESSED) {
        _handleHeatingTrigger(!heater.isHeating());
        return;
    }

    if (event.button == FIRE && event.type == HOLD_ONCE) {
        _handleHeatingTrigger(true);
        ds.targetTemperature.set(200);
        return;
    }
    if (event.button == FIRE && event.type == RELEASE) {
        _handleHeatingTrigger(false);
        ds.targetTemperature.set(ds.currentCycle.get() == 1 ? ds.targetTemperatureCycle1.get() : ds.targetTemperatureCycle2.get());
        return;
    }

    if (event.button == CENTER && event.type == PRESSED) {
        HeaterCycle::nextCycle();
        if (triggeredTwice(200) && state.heater.isHeating) ds.zvsDebug.set(!ds.zvsDebug.get());
        dirty();
        return;
    }

    if (event.button == LEFT || event.button == RIGHT) {
        uint8_t delta = event.button == LEFT ? -10 : 10;
        ds.power.update([delta](uint8_t val) { 
            uint8_t newVal = val + delta;
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
    dirty();
}

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
    int height = thin ? 20 : 50;
    int radius = 16;
    uint8_t _bgColor;
    uint8_t _textColor;

    if (!invert) {
        _bgColor = bgColor;
        _textColor = textColor;
    } else {
        _bgColor = COLOR_HIGHLIGHT;
        _textColor = textColor;
    }

        if (invert) {
            sprite->fillSmoothRoundRect(x, y, width, height, radius, COLOR_TEXT_SECONDARY, _textColor);
            sprite->fillSmoothRoundRect(x+2, y+2, width-4, height-4, radius - 2, _bgColor, _textColor);
        } else {
            sprite->fillSmoothRoundRect(x, y, width, height, radius, _bgColor, _textColor);

        }

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
        if (integer > 0) {
            sprintf(consumptionStr, "%d.%02dg", integer, decimal);
        }
        else {
            sprintf(consumptionStr, ".%02dg", decimal);
        }

        // Verbrauchswert rechts ausgerichtet
        sprite->setTextDatum(MR_DATUM);
        sprite->setFreeFont(thin ? &FreeSans9pt7b : &FreeSans18pt7b);
        sprite->drawString(consumptionStr, x + width - 12, y + height / 2);
}
