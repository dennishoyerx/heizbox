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
#include "utils/Format.h"

#include <ObservableMenuItem.h>

FireScreen::FireScreen(HeaterController &hc) : heater(hc) {
    auto& ds = DeviceState::instance();
    auto& hs = HeaterState::instance();

    bindTo(state.consumption.session, ds.sessionConsumption);
    bindTo(state.consumption.today, ds.todayConsumption);
    bindTo(state.consumption.yesterday, ds.yesterdayConsumption);
    
    redrawOn(ds.currentCycle);
    redrawOn(hs.tempLimit);
    redrawOn(hs.temp);
    redrawOn(hs.tempIR);
    redrawOn(hs.tempK);
    redrawOn(hs.isHeating);
    redrawOn(hs.power);

    hs.isHeating.addListener([&](bool isHeating) {
        if (!isHeating) _ui->clear();
    });
    
    menu.addItem(std::make_unique<ObservableValueItem<float>>(
        "tempLimit",
        hs.tempLimit,
        0.0f, 100.0f, 0.5f,
        [](const float& v){ char buf[16]; snprintf(buf, sizeof(buf), "%.1fC", v); return std::string(buf); }
    ));
    
    menu.addItem(std::make_unique<ObservableValueItem<int>>(
        "Power",
        hs.power,
        0, 100, 10,
        [](const int& v){ return std::to_string(v) + std::string("%"); }
    ));

    /*
    String KLog;
    hs.tempK.addListener([&](uint16_t val) {
        if (!heater.isHeating()) return;
        float t = heater.getElapsedTime() / 1000.0f;   // Sekunden
        if (KLog.length() > 0) KLog += ",";
        KLog += String(t, 1) + ":" + String(val);
    });
    
    String IRLog;
    hs.tempIR.addListener([&](uint16_t val) {
        if (!heater.isHeating()) return;
        float t = heater.getElapsedTime() / 1000.0f;   // Sekunden
        if (IRLog.length() > 0) IRLog += ",";
        IRLog += String(t, 1) + ":" + String(val);
    });

    hs.isHeating.addListener([&](bool isHeating) {
        if (!isHeating) {
            logPrint("IRLog", IRLog);
            logPrint("KLog", KLog);

            IRLog = "";
            KLog = "";
        }
    });*/

}


void FireScreen::draw() {
    auto& hs = HeaterState::instance();

    if (hs.isHeating) {
        HeatUI::render(_ui, heater.getZVSDriver());
        return;
    }

    _ui->withSurface(200, 70, 15, 122, [this](RenderSurface& s) {
        const IMenuItem* cur = menu.current();
        const IMenuItem* left = menu.at((menu.index() + menu.count() - 1) % (menu.count() ? menu.count() : 1));
        const IMenuItem* right = menu.at((menu.index() + 1) % (menu.count() ? menu.count() : 1));

        s.text(0, 0, cur->name());
        s.text(0, 30, cur->valueString());
    });

    _ui->withSurface(48, 48, 232, 130, [this](RenderSurface& s) {
        if (HeaterCycle::is(1)) {
            s.sprite->drawBitmap(0, 0, image_cap_fill_48, 48, 48, COLOR_TEXT_PRIMARY);
        } else {
            s.sprite->fillRect(0, 0, 48, 48, COLOR_BG);
        }
    });

    // Current Temp
    _ui->withSurface(88, 80, 0, 45, {
        {"temp", hs.temp},
        {"irTemp", hs.tempIR},
        {"thermoTemp", hs.tempK}
    }, [&hs](RenderSurface& s) {
        s.sprite->drawBitmap(-5, 0, image_temp_40, 40, 40, COLOR_TEXT_PRIMARY);
        s.text(30, 0, String(hs.temp), TextSize::lg);
        s.text(30, 30, String(hs.tempK), TextSize::lg);
        s.text(30, 60, String(hs.tempIR), TextSize::md);
    });

    // Target Temp
    _ui->withSurface(104, 50, 84, 45, {
        {"targetTemp", hs.tempLimit},
    }, [&hs](RenderSurface& s) {
        s.sprite->drawBitmap(0, 0, image_target_40, 40, 40, COLOR_TEXT_PRIMARY);
        s.text(40, 6, String(hs.tempLimit), TextSize::lg);
    });

    // Power
    _ui->withSurface(100, 40, 192, 45, {
        {"power", hs.power}
    }, [&hs](RenderSurface& s) {
        s.sprite->drawBitmap(-10, 0, image_power_40, 40, 40, COLOR_TEXT_PRIMARY);
        s.text(30, 6, String(hs.power), TextSize::lg);
    });
    
    // Consumption
    _ui->withSurface(250, 50, 15, 190, {
        {"isHeating", hs.isHeating},
        {"consumption", state.consumption.session},
        {"todayConsumption", state.consumption.today},
        {"currentCycle", HeaterCycle::current()}
    }, [this](RenderSurface& s) {
        drawStats(s, 0, 0, "Session", formatConsumption(state.consumption.session));
        drawStats(s, 80, 0, "Heute", formatConsumption(state.consumption.today));
        drawStats(s, 160, 0, "Gestern", formatConsumption(state.consumption.yesterday));
    });
return;
    // Seperator
    _ui->withSurface(280, 1, 0, 95, [this](RenderSurface& s) {
        s.sprite->drawRect(0, 0, 280, 1, COLOR_BG_2);
    });

}

void FireScreen::update() {
    auto& hs = HeaterState::instance();
     if (hs.isHeating) dirty();
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

bool button(InputEvent event,
            std::initializer_list<InputButton> buttons,
            std::initializer_list<InputEventType> types) {
    bool bMatch = false;
    for (auto b : buttons) {
        if (event.button == b) {
            bMatch = true;
            break;
        }
    }

    if (!bMatch) return false;

    for (auto t : types) {
        if (event.type == t) {
            return true;
        }
    }

    return false;
}

void FireScreen::handleInput(InputEvent event) {
    auto& ds = DeviceState::instance();
    auto& hs = HeaterState::instance();

    static uint32_t _temp;
    if (button(event, {FIRE}, {PRESSED})) {
        _handleHeatingTrigger(!heater.isHeating());
        return;
    }

    if (button(event, {FIRE}, {HOLD_ONCE})) {
        _handleHeatingTrigger(true);
        hs.tempLimit.set(HeaterConfig::MAX_TEMPERATURE);
        return;
    }
    if (button(event, {FIRE}, {RELEASE})) {
        _handleHeatingTrigger(false);
        hs.tempLimit.set(HeaterCycle::is(1) ? ds.targetTemperatureCycle1.get() : ds.targetTemperatureCycle2.get());
        return;
    }
    
    if (button(event, {CENTER}, {PRESS})) {
        HeaterCycle::next();
        return;
    }

    if (button(event, {LEFT}, {PRESS})) {
        menu.prevOption();
        dirty();
    }
    if (button(event, {RIGHT}, {PRESS})) {
        menu.nextOption();
        dirty();
    }

    if (button(event, {UP}, {PRESS, HOLD})) {
        menu.increment();
    }
    if (button(event, {DOWN}, {PRESS, HOLD})) {
        menu.decrement();
    }

    return;
    if (button(event, {LEFT, RIGHT}, {PRESS})) {
        uint8_t delta = event.button == LEFT ? -1 : 1;
        ds.irEmissivity.update([delta](uint8_t val) { 
            uint8_t newVal = val + delta;
            if (newVal == 0) newVal = 1;
            if (newVal == 101) newVal = 100;
            return newVal; 
        });
        return;
    }

    
    if (button(event, {UP, DOWN}, {PRESS, HOLD})) {
        float delta = event.button == UP ? 1 : -1;
        
        PersistedObservable<uint16_t>* cycleTemp = HeaterCycle::is(1) ? &ds.targetTemperatureCycle1 : &ds.targetTemperatureCycle2;
        uint16_t temp = cycleTemp->update([delta](uint16_t val) { return val + delta; });

        hs.tempLimit.set(temp);
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


void FireScreen::_handleHeatingTrigger(bool shouldStartHeating) {
    if (shouldStartHeating) {
        heater.startHeating();
    } else if (heater.isHeating()) {
        heater.stopHeating(false);
    }
    dirty();
}
