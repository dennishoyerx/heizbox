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

#include "utils/Logger.h"
#include "utils/Format.h"

#include <Wire.h>
#include "utils/Logger.h"

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
    auto& hs = HeaterState::instance();
    static uint32_t lastDirty = 0; // Zeitpunkt des letzten dirty-Aufrufs
    uint32_t now = millis();

    if (!hs.isHeating) {
        // alle 200 ms, wenn nicht heizt
        if (now - lastDirty >= 200) {
            dirty();
            lastDirty = now;
        }
    } else {
        // beim Heizen: sofort dirty() aufrufen
        dirty();
        lastDirty = now; // optional, falls Intervall-Logik später noch wichtig
    }
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

    if (input(event, {FIRE}, {PRESSED, HOLD}) && input(event, {CENTER}, {PRESSED, HOLD})) {
        bool locked = ds.locked.set(!ds.locked);
        static uint8_t initialBrightness;
        if (locked) initialBrightness = ds.brightness;
        ds.brightness.set(locked ? 10 : initialBrightness ? initialBrightness : 100, false);
        return;
    }
    if (ds.locked) return;

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
    
    // CENTER: either cycle or trigger selected menu action (IR Cal A/B/Clear)
    if (input(event, {CENTER}, {PRESSED})) {
        Wire.begin(InputConfig::PCF8574::SDA, InputConfig::PCF8574::SCL);
        for (uint8_t addr = 1; addr < 127; addr++) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {char buf[6];
snprintf(buf, sizeof(buf), "0x%02X", addr);
    logPrint(buf);
  }
}
        const IMenuItem* cur = menu.current();
        String curName = cur ? String(cur->name()) : String();

        if (curName == "IR Cal A") {
            // trigger calibration A
            if (!heater.isHeating()) {
                Serial.println("IR click ignored: not heating.");
                showOverlay("IR click ignored: not heating", 1200);
            } else {
                uint16_t actualTemp = hs.irCalActualA;
                Serial.printf("Menu: Storing IR Cal A for actual=%u\n", actualTemp);
                int res = heater.markIRClick(actualTemp);
                if (res == 0) {
                    showOverlay("IR click failed (no valid samples)", 1500);
                } else {
                    uint16_t measured = hs.irCalMeasuredA.get();
                    showOverlay(String("IR Cal A: ") + String(measured) + "°", 1500);
                }

                menu.nextOption();
            }
            dirty();
            return;
        } else if (curName == "IR Cal B") {
            // trigger calibration B
            if (!heater.isHeating()) {
                Serial.println("IR click ignored: not heating.");
                showOverlay("IR click ignored: not heating", 1200);
            } else {
                uint16_t actualTemp = hs.irCalActualB;
                Serial.printf("Menu: Storing IR Cal B for actual=%u\n", actualTemp);
                int res = heater.markIRClick(actualTemp);
                if (res == 0) {
                    showOverlay("IR click failed (no valid samples)", 1500);
                } else {
                    uint16_t measured = hs.irCalMeasuredB.get();
                    showOverlay(String("IR Cal B: ") + String(measured) + "°", 1500);
                }
                menu.prevOption();
            }
            dirty();
            return;
        } else if (curName == "IR Cal Clear") {
            heater.clearIRCalibration();
            showOverlay("IR Calibration cleared", 1400);
            dirty();
            return;
        }

        // default CENTER behavior (cycle)
        HeaterCycle::next();
        return;
    }

    if (input(event, {LEFT}, {PRESS})) {
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