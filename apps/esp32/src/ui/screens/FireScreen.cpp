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
    
    bindMultiple(hs.isHeating, hs.power, hs.cycle, 
        hs.tempLimit, hs.temp, hs.tempIR, hs.tempK, 
        hs.tempSensorOffTime, hs.tempSensorReadInterval, 
        hs.irEmissivity, hs.tempCorrection
    );

    hs.isHeating.addListener([&](bool isHeating) {
        if (!isHeating) _ui->clear();
    });
    
    menu.addItem(std::make_unique<ObservableValueItem<uint16_t>>(
        "Temp Cycle 1", ds.targetTemperatureCycle1, 100, 260, 1,
        [](const uint16_t& v){ return (String) v + "°"; }
    ));
    
    menu.addItem(std::make_unique<ObservableValueItem<uint16_t>>(
        "Temp Cycle 2", ds.targetTemperatureCycle2, 100, 260, 1,
        [](const uint16_t& v){ return (String) v + "°"; }
    ));
    
    menu.addItem(std::make_unique<ObservableValueItem<int8_t>>(
        "Temp Correction", hs.tempCorrection, -50, 50, 1,
        [](const int8_t& v){ return (String) v + "°"; }
    ));

    menu.addItem(std::make_unique<ObservableValueItem<uint8_t>>(
        "IR Emissivity", hs.irEmissivity, 0, 100, 1,
        [](const uint8_t& v){ return (String) v + "%"; }
    ));

    menu.addItem(std::make_unique<ObservableValueItem<uint32_t>>(
        "Temp Sensor Off Time", hs.tempSensorOffTime, 0, 220, 20,
        [](const uint32_t& v){ return (String) v + "ms"; }
    ));

    menu.addItem(std::make_unique<ObservableValueItem<uint32_t>>(
        "Temp Read Interval", hs.tempSensorReadInterval, 0, 220, 20,
        [](const uint32_t& v){ return (String) v + "ms"; }
    ));

    menu.addItem(std::make_unique<ObservableValueItem<uint8_t>>(
        "Power", hs.power, 0, 100, 10,
        [](const uint8_t& v){ return (String) v + "%"; }
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
        HeatUI::render(_ui, heater.getZVSDriver(), &menu);
        

    /*_ui->withSurface(200, 60, 15, 175, [this](RenderSurface& s) {
        const IMenuItem* cur = menu.current();
        const IMenuItem* left = menu.at((menu.index() + menu.count() - 1) % (menu.count() ? menu.count() : 1));
        const IMenuItem* right = menu.at((menu.index() + 1) % (menu.count() ? menu.count() : 1));
        s.sprite->fillRect(0, 0, 200, 60, COLOR_BG);
        s.text(0, 0, cur->name());
        s.text(0, 30, cur->valueString());
    });*/
        return;
    }

    _ui->withSurface(200, 60, 15, 130, [this](RenderSurface& s) {
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

void FireScreen::handleInput(InputEvent event) {
    auto& ds = DeviceState::instance();
    auto& hs = HeaterState::instance();

    static uint32_t _temp;
    if (input(event, {FIRE}, {PRESSED})) {
        _handleHeatingTrigger(!heater.isHeating());
        return;
    }

    if (input(event, {FIRE}, {HOLD_ONCE})) {
        _handleHeatingTrigger(true);
        hs.tempLimit.set(HeaterConfig::MAX_TEMPERATURE);
        return;
    }
    if (input(event, {FIRE}, {RELEASE})) {
        _handleHeatingTrigger(false);
        hs.tempLimit.set(HeaterCycle::is(1) ? ds.targetTemperatureCycle1.get() : ds.targetTemperatureCycle2.get());
        return;
    }
    
    if (input(event, {CENTER}, {PRESSED})) {
        HeaterCycle::next();
        return;
    }

    if (input(event, {LEFT}, {PRESS, PRESSED, HOLD})) {
        menu.prevOption();
        dirty();
    }
    if (input(event, {RIGHT}, {PRESS})) {
        menu.nextOption();
        dirty();
    }

    if (input(event, {UP}, {PRESS, HOLD})) {
        menu.increment();
        dirty();
    }
    if (input(event, {DOWN}, {PRESS, HOLD})) {
        menu.decrement();
        dirty();
    }
}


void FireScreen::_handleHeatingTrigger(bool shouldStartHeating) {
    if (shouldStartHeating) {
        heater.startHeating();
    } else if (heater.isHeating()) {
        heater.stopHeating(false);
    }
    dirty();
}
