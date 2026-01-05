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
        "Temperature", hs.tempLimit, 100, 260, 1,
        [](const uint16_t& v){ return (String) v + "°"; }
    ));
    
    
    menu.addItem(std::make_unique<ObservableValueItem<uint8_t>>(
        "IR Emissivity", hs.irEmissivity, 0, 100, 1,
        [](const uint8_t& v){ return (String) v + "%"; }
    ));

    /*
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
    */
}
/*
struct Column {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
};

class Row {
    public:
    Row(int8_t cols, int16_t w, int16_t h, int16_t x, int16_t y);

    Column col(int8_t col) {
        Column c;
       // c.x = 
       // c.y = cols[col].y;
        c.w = cols[col].w;
        c.h = h;
        return c;
    };

    private:
    int8_t cols;
    int16_t w;
    int16_t h;
    int16_t x;
    int16_t y;
    struct col {
        int16_t w;
        int16_t h;
    };
};*/

struct TemperatureProps {
    int limit;
    int ktyp;
    int ir;
};

void TemperatureComponent(RenderSurface& s, TemperatureProps props) {
        int padding = 16;
        s.sprite->fillRect(0, 0, s.width(), s.height(), COLOR_BG_2);
        s.text(16, padding + 0, "KTyp", TextSize::sm);
        s.text(16, padding + 24, String(props.ktyp), TextSize::blg);

        String tempIR = props.ir > 500 ? "--" : String(props.ir);
        
        s.text(88, padding + 0, "IR", TextSize::sm);
        s.text(88, padding + 24, tempIR, TextSize::blg);
        
        s.text(160, padding + 0, "Limit", TextSize::sm);
        s.text(160, padding + 24, String(props.limit), TextSize::bxl);
    };

void FireScreen::draw() {
    auto& hs = HeaterState::instance();

    if (hs.isHeating) {
        HeatUI::render(_ui, heater.getZVSDriver(), &menu);
        return;
    }

    _ui->withSurface(200, 60, 15, 100, [this](RenderSurface& s) {
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
    _ui->withSurface(280, 88, 0, 0, {
        {"targetTemp", hs.tempLimit},
        {"temp", hs.temp},
        {"irTemp", hs.tempIR},
        {"thermoTemp", hs.tempK}
    }, [&hs](RenderSurface& s) {
        s.sprite->fillRect(0, 0, s.width(), s.height(), COLOR_BG_2);
        HeatUI::Temperature(s);
    }); 

    // Consumption
    _ui->withSurface(200, 50, 80, 190, {
        {"isHeating", hs.isHeating},
        {"consumption", state.consumption.session},
        {"todayConsumption", state.consumption.today},
        {"currentCycle", HeaterCycle::current()}
    }, [this](RenderSurface& s) {
        s.sprite->fillRect(0, 0, s.width(), s.height(), COLOR_BG_2);
        drawStats(s, 0, 0, "Session", formatConsumption(state.consumption.session));
        drawStats(s, 80, 0, "Heute", formatConsumption(state.consumption.today));
        drawStats(s, 148, 0, "Gestern", formatConsumption(state.consumption.yesterday));
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
        hs.tempLimit.set(HeaterCycle::is(1) ? hs.tempLimitCycle1: hs.tempLimitCycle2);
        return;
    }
    
    if (input(event, {CENTER}, {PRESSED})) {
        HeaterCycle::next();
        return;
    }

    if (input(event, {LEFT}, {PRESSED})) {
        menu.prevOption();
        dirty();
    }
    if (input(event, {RIGHT}, {PRESSED})) {
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
