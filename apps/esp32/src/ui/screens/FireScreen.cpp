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
#include "heater/Presets.h"
#include "driver/Audio.h"

#include "utils/Logger.h"
#include "utils/Format.h"

#include <Wire.h>
#include "utils/Logger.h"

#include <ObservableMenuItem.h>

#include <Time.hpp>
#include <Calibration.h>

FireScreen::FireScreen(HeaterController &hc) : heater(hc) {
    auto& ds = DeviceState::instance();
    auto& hs = HeaterState::instance();

    bindTo(state.consumption.session, ds.consumption.session);
    bindTo(state.consumption.today, ds.consumption.today);
    bindTo(state.consumption.yesterday, ds.consumption.yesterday);
    
    bindMultiple(hs.isHeating, hs.power, hs.cycle, 
        hs.tempLimit, hs.temp, hs.tempIR, hs.tempK, 
        hs.tempSensorOffTime, hs.tempSensorReadInterval, 
        hs.irEmissivity, hs.tempCorrection
    );

    hs.isHeating.addListener([&](bool isHeating) {
        if (!isHeating) _ui->clear();
        manager->setStatusbarVisible(!isHeating);


    });

    menu.addItem(std::make_unique<ObservableValueItem<uint16_t>>(
        "Temperature", hs.tempLimit, 100, 260, 1,
        [&hs](const uint16_t& v){ 
            if (hs.mode == HeaterMode::PRESET) return (String) v + " (" + TempPresets[hs.currentPreset].name  + ")";
            return (String) v + "°";
         }
    ));

    menu.addItem(std::make_unique<ObservableValueItem<uint8_t>>(
        "Preset", hs.currentPreset, 0, 3, 1,
        [](const uint8_t& v){ return (String) TempPresets[v].name; }
    ));

    menu.addItem(std::make_unique<ObservableValueItem<uint8_t>>(
        "Power", hs.power, 10, 100, 10,
        [](const uint8_t& v){ return (String) v + "%"; }
    ));

    // --- IR calibration menu actions (select + CENTER to trigger) ---
    static Observable<uint8_t> menuIRCalA{0};
    static Observable<uint8_t> menuIRCalB{0};
    static Observable<uint8_t> menuIRCalClear{0};

    menu.addItem(std::make_unique<ObservableValueItem<uint8_t>>(
        "IR Cal A", menuIRCalA, 0, 1, 1,
        [](const uint8_t& v){ return v == 0 ? "" : "Press"; }
    ));
    menu.addItem(std::make_unique<ObservableValueItem<uint8_t>>(
        "IR Cal B", menuIRCalB, 0, 1, 1,
        [](const uint8_t& v){ return v == 0 ? "" : "Press"; }
    ));
}

// Overlay state for confirmation messages
static String s_overlayText = "";
static uint32_t s_overlayUntil = 0;
static const uint32_t OVERLAY_DEFAULT_MS = 1500;

static void showOverlay(const String &text, uint32_t durationMs = OVERLAY_DEFAULT_MS) {
    s_overlayText = text;
    s_overlayUntil = millis() + durationMs;
}

void FireScreen::draw() {
    auto& hs = HeaterState::instance();

    // If heating, use HeatUI (this also updates frequently)
    if (hs.isHeating) {
        HeatUI::render(_ui, heater.getZVSDriver(), &menu);

        // Draw overlay on top if active
        if (s_overlayUntil != 0 && millis() < s_overlayUntil) {
            _ui->withSurface(240, 30, 15, 180, [](RenderSurface& s) {
                s.sprite->fillRect(0, 0, s.width(), s.height(), COLOR_BG_2);
                s.text(4, 8, s_overlayText);
            });
        }
        return;
    }

    _ui->withSurface(200, 60, 15, 100, [this](RenderSurface& s) {
        const IMenuItem* cur = menu.current();
        const IMenuItem* left = menu.at((menu.index() + menu.count() - 1) % (menu.count() ? menu.count() : 1));
        const IMenuItem* right = menu.at((menu.index() + 1) % (menu.count() ? menu.count() : 1));

        s.text(0, 0, cur->name());
        s.text(0, 30, cur->valueString());
    });

    // Current Temp
    _ui->withSurface(280, 88, 0, 0, [&hs](RenderSurface& s) {
        s.sprite->fillRect(0, 0, s.width(), s.height(), COLOR_BG_2);
        HeatUI::Temperature(s);
        HeatUI::Cycle(s);
    }); 

    // Consumption
    _ui->withSurface(200, 50, 96, 190, {
        {"isHeating", hs.isHeating},
        {"consumption", state.consumption.session},
        {"todayConsumption", state.consumption.today},
        {"currentCycle", HeaterCycle::current()}
    }, [this](RenderSurface& s) {
        s.sprite->fillRect(0, 0, s.width(), s.height(), COLOR_BG_2);
        drawStats(s, 0, 0, "Session", formatConsumption(state.consumption.session));
        drawStats(s, 72, 0, "Heute", formatConsumption(state.consumption.today));
        drawStats(s, 132, 0, "Gestern", formatConsumption(state.consumption.yesterday));
    });

    // Draw overlay if active (when not heating)
    if (s_overlayUntil != 0 && millis() < s_overlayUntil) {
        _ui->withSurface(240, 30, 15, 180, [](RenderSurface& s) {
            s.sprite->fillRect(0, 0, s.width(), s.height(), COLOR_BG_2);
            s.text(4, 8, s_overlayText);
        });
    }
return;
    // Seperator
    _ui->withSurface(280, 1, 0, 95, [this](RenderSurface& s) {
        s.sprite->drawRect(0, 0, 280, 1, COLOR_BG_2);
    });
}


void FireScreen::update() {
    if (heater.isHeating()) dirty();
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

class ManualControl {
    static bool active;

public:
    static bool isActive() { return active; }
    static void setActive(bool val) { active = val; }

};

class Every {
    uint32_t _interval;
    uint32_t _last;
public:
    Every(uint32_t interval) : _interval(interval), _last(0) {}
    bool operator()() {
        uint32_t now = millis();
        if (now - _last >= _interval) {
            _last = now;
            return true;
        }
        return false;
    }

};


void FireScreen::handleInput(InputEvent event) {
    auto& ds = DeviceState::instance();
    auto& hs = HeaterState::instance();

    if (input(event, {FIRE}, {PRESSED, HOLD}) && input(event, {CENTER}, {PRESSED, HOLD})) {
        bool locked = ds.locked.set(!ds.locked);
        static uint8_t initialBrightness;
        if (locked) initialBrightness = ds.display.brightness;
        ds.display.brightness.set(locked ? 10 : initialBrightness ? initialBrightness : 100, false);
        return;
    }
    if (ds.locked) return;

    if (input(event, {FIRE}, {PRESSED})) return _handleHeatingTrigger(!heater.isHeating());
    else if (input(event, {FIRE}, {HOLD_ONCE})) {
        hs.tempLimit.set(HeaterConfig::MAX_TEMPERATURE);
        _handleHeatingTrigger(true);
        return;
    } else if (input(event, {FIRE}, {RELEASE})) {
        _handleHeatingTrigger(false);
        if (hs.mode == HeaterMode::PRESET) hs.tempLimit.set(Presets::getPresetTemp(hs.currentPreset));
        else hs.tempLimit.set(HeaterCycle::is(1) ? hs.tempLimitCycle1: hs.tempLimitCycle2);
        return;
    }

    if (input(event, {ROTARY_ENCODER}, {PRESSED})) {
        HeaterCycle::next();
        if (HeaterCycle::is(1)) Audio::beepCycleDown();
        else Audio::beepCycleUp();
        return;
    }
    
    // CENTER: either cycle or trigger selected menu action (IR Cal A/B/Clear)
    if (input(event, {CENTER}, {PRESSED})) {
        const IMenuItem* cur = menu.current();
        String curName = cur ? String(cur->name()) : String();
        if (curName == "Temperature") {
            if (hs.mode == HeaterMode::PRESET) {
                Presets::setPresetTemp(hs.currentPreset, hs.tempLimit.get());
                    showOverlay("Preset gespeichert.", 1500);
                Audio::beepSuccess();
            }
        } else if (curName == "IR Cal A") {
            // trigger calibration A
            if (!heater.isHeating()) {
                Serial.println("IR click ignored: not heating.");
                showOverlay("IR click ignored: not heating", 1200);
                    Audio::beepError();
            } else {
                //uint16_t measured = heater.getIRTempSensor()->getCalibration()->setMeasurement(IRCalibration::Point::A, hs.irCalActualA);
                uint16_t actualTemp = hs.irCalActualA;
                Serial.printf("Menu: Storing IR Cal A for actual=%u\n", actualTemp);
                int res = heater.markIRClick(actualTemp);
                if (res == 0) {
                    showOverlay("IR click failed (no valid samples)", 1500);
                    Audio::beepError();
                } else {
                    uint16_t measured = hs.irCalMeasuredA.get();
                    showOverlay(String("IR Cal A: ") + String(measured) + "°", 1500);
                    Audio::beepSuccess();
                }

                menu.nextOption();
            }
            return;
        } else if (curName == "IR Cal B") {
            // trigger calibration B
            if (!heater.isHeating()) {
                Serial.println("IR click ignored: not heating.");
                showOverlay("IR click ignored: not heating", 1200);
                    Audio::beepError();
            } else {
                //uint16_t measured = heater.getIRTempSensor()->getCalibration()->setMeasurement(IRCalibration::Point::B, hs.irCalActualB);
                uint16_t actualTemp = hs.irCalActualB;
                Serial.printf("Menu: Storing IR Cal B for actual=%u\n", actualTemp);
                int res = heater.markIRClick(actualTemp);
                if (res == 0) {
                    showOverlay("IR click failed (no valid samples)", 1500);
                    Audio::beepError();
                } else {
                    uint16_t measured = hs.irCalMeasuredB.get();
                    showOverlay(String("IR Cal B: ") + String(measured) + "°", 1500);
                    Audio::beepSuccess();
                }
                menu.prevOption();
            }
            return;
        } else if (curName == "IR Cal Clear") {
            //heater.getIRTempSensor()->getCalibration()->clear();
            heater.clearIRCalibration();
            showOverlay("IR Calibration cleared", 1400);
            Audio::beepWarning();
            return;
        }
        return;
    }

    if (input(event, {LEFT}, {PRESS})) {
        menu.prevOption();
        Audio::beepMenu();
    }
    if (input(event, {RIGHT}, {PRESS})) {
        menu.nextOption();
        Audio::beepMenu();
    }

    
    if (input(event, {UP}, {PRESS, HOLD})) {
        menu.increment();
        Audio::beepMenu();
    }
    
    if (input(event, {DOWN}, {PRESS, HOLD})) {
        menu.decrement();
        Audio::beepMenu();
    }

    if (input(event, {ROTARY_ENCODER}, {ROTARY_CW, ROTARY_CCW})) {
        int delta = event.type == ROTARY_CW ? 1 : -1;
        
        hs.tempLimit.set(hs.tempLimit.get() + delta);
        Audio::rotaryTurn(ROTARY_CW == event.type);
    }
}


void FireScreen::_handleHeatingTrigger(bool shouldStartHeating) {
    if (shouldStartHeating) {
        heater.startHeating();
    } else if (heater.isHeating()) {
        heater.stopHeating(false);
    }
}