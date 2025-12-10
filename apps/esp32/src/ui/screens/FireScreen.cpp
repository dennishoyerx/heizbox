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
 #include <Wire.h>
#include <utility>
#include "heater/HeaterState.h"

#include "utils/Logger.h"

FireScreen::FireScreen(HeaterController &hc) : heater(hc) {
    auto& ds = DeviceState::instance();
    auto& hs = HeaterState::instance();

    bindTo(state.consumption.session, ds.sessionConsumption);
    bindTo(state.consumption.today, ds.todayConsumption);
    bindTo(state.consumption.yesterday, ds.yesterdayConsumption);
    bindTo(state.heater.targetTemp, ds.targetTemperature);
    bindTo(state.heater.power, ds.power);
    bindTo(state.heater.currentCycle, ds.currentCycle);
    bindTo(state.heater.isHeating, hs.isHeating);
    bindTo(state.heater.thermoTemp, hs.tempK);
    bindTo(state.heater.irTemp, hs.tempIR);
}

String formatConsumption(float consumption) {
        char consumptionStr[10];
        int integer = (int)consumption;
        int decimal = ((int)(consumption * 100 + 0.5f)) % 100;
        if (integer > 0) {
            sprintf(consumptionStr, "%d.%02dg", integer, decimal);
        }
        else {
            sprintf(consumptionStr, ".%02dg", decimal);
        }
    return (String) consumptionStr;
}

void drawStats(RenderSurface& s, int x, int y, String label, String value) {
    //s.sprite->setTextDatum(MC_DATUM);
    s.text(x, y, value);
    s.text(x, y + 24, label, TextSize::sm);
}

void FireScreen::draw() {
    if (state.heater.isHeating) {
        ZVSDriver* zvs = heater.getZVSDriver();
        HeatUI::render(_ui, state.heater, zvs);

        return;
    }

    _ui->withSurface(48, 48, 15, 110, [this](RenderSurface& s) {
        if (HeaterCycle::is(1)) {
            s.sprite->drawBitmap(0, 0, image_cap_fill_48, 48, 48, COLOR_TEXT_PRIMARY);
        } else {
            s.sprite->fillRect(0, 0, 48, 48, COLOR_BG);
        }
    });
    
    // Consumption
    _ui->withSurface(250, 50, 15, 190, {
        {"isHeating", state.heater.isHeating},
        {"consumption", state.consumption.session},
        {"todayConsumption", state.consumption.today},
        {"currentCycle", state.heater.currentCycle}
    }, [this](RenderSurface& s) {
        drawStats(s, 0, 0, "Session", formatConsumption(state.consumption.session));
        drawStats(s, 80, 0, "Heute", formatConsumption(state.consumption.today));
        drawStats(s, 160, 0, "Gestern", formatConsumption(state.consumption.yesterday));
        //FireScreen::drawSessionRow(s.sprite, "Session", state.consumption.session, 0, COLOR_BG_2, COLOR_BG_2, COLOR_TEXT_PRIMARY, (state.heater.currentCycle == 1));
        //FireScreen::drawSessionRow(s.sprite, "Heute", state.consumption.today, 55, COLOR_BG_3, COLOR_BG_2, COLOR_TEXT_PRIMARY);
        //FireScreen::drawSessionRow(s.sprite, "Gestern", state.consumption.yesterday, 105, COLOR_BG, COLOR_BG, COLOR_TEXT_PRIMARY, false, true);
    });

    // Current Temp
    _ui->withSurface(88, 50, 0, 45, {
        {"irTemp", hs().tempIR},
        {"thermoTemp", hs().tempK}
    }, [this](RenderSurface& s) {
        s.sprite->drawBitmap(-5, 0, image_temp_40, 40, 40, COLOR_TEXT_PRIMARY);
        s.text(30, -4, String(hs().tempK), TextSize::lg);
        s.text(30, 26, String(hs().tempIR), TextSize::md);
    });

    // Target Temp
    _ui->withSurface(104, 50, 84, 45, {
        {"targetTemp", state.heater.targetTemp},
        {"currentCycle", state.heater.currentCycle}
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
    if (hs().isHeating) {
        state.heater.elapsedSeconds = heater.getElapsedTime() / 1000;
        state.heater.progress = (float)state.heater.temp / state.heater.targetTemp;

        if (state.heater.progress > 1.0f) state.heater.progress = 1.0f;

        if (hs().tempK > state.heater.targetTemp) {
            _handleHeatingTrigger(false);
            dirty();
        }

        static uint32_t lastSecond = 0;
        if (hs().timer != lastSecond) {
            lastSecond = hs().timer;
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
        
        PersistedObservable<uint16_t>* cycleTemp = HeaterCycle::is(1) ? &ds.targetTemperatureCycle1 : &ds.targetTemperatureCycle2;
        uint16_t temp = cycleTemp->update([delta](uint16_t val) { return val + delta; });

        ds.targetTemperature.set(temp);
        return;
    }

    if (event.button == CENTER && event.type == PRESS) {
        HeaterCycle::next();
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
        ds.targetTemperature.set(HeaterConfig::MAX_TEMPERATURE);
        return;
    }
    if (event.button == FIRE && event.type == RELEASE) {
        _handleHeatingTrigger(false);
        ds.targetTemperature.set(HeaterCycle::is(1) ? ds.targetTemperatureCycle1.get() : ds.targetTemperatureCycle2.get());
        return;
    }

    /*if (event.button == LEFT || event.button == RIGHT) {
        uint8_t delta = event.button == LEFT ? -10 : 10;
        ds.power.update([delta](uint8_t val) { 
            uint8_t newVal = val + delta;
            if (newVal < 30) newVal = 30;
            if (newVal > 100) newVal = 100;
            return newVal; 
        });
        return;
    }*/
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
