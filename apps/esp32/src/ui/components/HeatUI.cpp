#include "ui\components\HeatUI.h"
#include "bitmaps.h"
#include "ui/ColorPalette.h"
#include "ui/base/Screen.h"
#include "ui/components/ZVSOscilloscope.h"
#include "heater/HeaterState.h"
#include "heater/HeaterCycle.h"
#include <algorithm>

float smoothProgress(float target) {
    static float smoothed = 0.0f;
    static uint32_t last = millis();

    uint32_t now = millis();
    float dt = (now - last) / 1000.0f;   // dt in Sekunden
    last = now;

    // Ziel: in exact DUTY_CYCLE_PERIOD_MS (1 s) zu ~95% am Ziel sein.
    // -> Klassischer RC-Filter: tau = Periode / 3
    // Dadurch 95% nach 1 Periode.
    const float tau = (HeaterConfig::DUTY_CYCLE_PERIOD_MS / 1000.0f) / 3.0f;

    float alpha = 1.0f - expf(-dt / tau);

    smoothed += (target - smoothed) * alpha;
    return smoothed;
}


void drawStats(RenderSurface& s, int x, int y, String label, String value) {
    //s.sprite->setTextDatum(MC_DATUM);
    s.text(x, y, value);
    s.text(x, y + 24, label, TextSize::sm);
}


void ZVSOscilloscopeUI(RenderSurface s, ZVSDriver* zvs) {
    static ZVSOscilloscope osc = ZVSOscilloscope(zvs, 280);
    osc.update();
    int h = s.height() /2;        // osc-Höhe
    int y = (s.height() - h) / 2; 
    osc.draw(s, 0, y, h);
}

void Background(RenderSurface s, float progress, uint8_t color) {
    int fillHeight = (int)(s.height() * progress);
    s.sprite->fillRect(s.left(), s.bottom() - fillHeight, s.width(), fillHeight, color);
}

void Timer(RenderSurface s, uint32_t time) {
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", time);

    s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
    s.sprite->setTextDatum(MC_DATUM);
    s.sprite->setTextSize(2);
    s.sprite->setFreeFont(&FreeSansBold18pt7b);
    s.sprite->drawString(timeStr, s.centerX(), s.centerY(), 1);
    s.sprite->setTextDatum(ML_DATUM);
}

void HeatUI::Temperature(RenderSurface s) {
    auto& hs = HeaterState::instance();

    s.sprite->drawBitmap(0, 15, image_temp_48, 48, 48, COLOR_TEXT_PRIMARY);
    s.text(45, 30, String(hs.temp), TextSize::bxl);
    
    s.text(32, 70, "k", TextSize::sm);
    s.text(45, 70, String(hs.tempK), TextSize::lg);

    s.text(32, 100, "ir", TextSize::sm);
    s.text(45, 100, String(hs.tempIR), TextSize::lg);

    s.text(150, 30, String(hs.tempLimit), TextSize::xl);
    s.text(150, 70, String(hs.irEmissivity / 100.0f), TextSize::sm);
}


void HeatUI::Cycle(RenderSurface s) {
    if (HeaterCycle::is(1)) s.sprite->drawBitmap(s.width() - 48, 20, image_cap_fill_48, 48, 48, COLOR_TEXT_PRIMARY);
}

void ZVSDebug(RenderSurface s, ZVSDriver* zvs) {
    if (!zvs)
        return;

    const auto& stats = zvs->getStats();

    // Current state
    const char* phaseStr = "";
    switch (zvs->getCurrentPhase()) {
        case ZVSDriver::Phase::OFF_IDLE:
            phaseStr = "IDLE";
            break;
        case ZVSDriver::Phase::ON_PHASE:
            phaseStr = "HEATING";
            break;
        case ZVSDriver::Phase::OFF_PHASE:
            phaseStr = "OFF";
            break;
        case ZVSDriver::Phase::SENSOR_WINDOW:
            phaseStr = "SENSOR";
            break;
    }

    uint8_t xOffset = 0;
    uint8_t yOffset = 110;
    s.sprite->setTextDatum(ML_DATUM);

    s.text(10 + xOffset, 10 + yOffset, String("Phase: ") + phaseStr, TextSize::md);
    s.text(10 + xOffset, 30 + yOffset, String("Power: ") + zvs->getPower() + "%", TextSize::md);
    s.text(10 + xOffset, 50 + yOffset, String("MOSFET: ") + (zvs->isPhysicallyOn() ? "ON" : "OFF"), TextSize::md);

    // Statistics
    s.text(10 + xOffset, 70 + yOffset, String("Cycles: ") + stats.cycleCount, TextSize::sm);
    s.text(10 + xOffset, 85 + yOffset, String("ON Time: ") + (stats.totalOnTime / 1000) + "s", TextSize::sm);
    s.text(10 + xOffset, 100 + yOffset, String("OFF Time: ") + (stats.totalOffTime / 1000) + "s", TextSize::sm);

    // Duty cycle calculation
    float dutyCycle = 0;
    if ((stats.totalOnTime + stats.totalOffTime) > 0) {
        dutyCycle = (float)stats.totalOnTime / (stats.totalOnTime + stats.totalOffTime) * 100;
    }
    s.text(130, 70 + yOffset, String("Duty: ") + String(dutyCycle, 1) + "%", TextSize::sm);
    s.text(130, 85 + yOffset, String("Temp Reads: ") + stats.tempMeasures, TextSize::sm);
}


void HeatUI::render(UI* _ui, ZVSDriver* zvs, MenuManager* menu) {
    auto& hs = HeaterState::instance();

    _ui->withSurface(280, 205, 0, 35, [&](RenderSurface& s) {
        s.sprite->setPaletteColor(15, ColorUtils::getTemperatureColor565(hs.temp, true));
        float progress = std::min(1.0f, (float)hs.temp / hs.tempLimit);
        Background(s, smoothProgress(progress), 15);
        
        if (DeviceState::instance().oscDebug) ZVSOscilloscopeUI(s, zvs);

        Timer(s, hs.timer);
        
        Cycle(s);
        Temperature(s);

        if (DeviceState::instance().zvsDebug) ZVSDebug(s, zvs);

        
        const IMenuItem* cur = menu->current();
        const IMenuItem* left = menu->at((menu->index() + menu->count() - 1) % (menu->count() ? menu->count() : 1));
        const IMenuItem* right = menu->at((menu->index() + 1) % (menu->count() ? menu->count() : 1));
        s.text(15, 160, cur->name());
        s.text(15, 190, cur->valueString());
    });
}; 