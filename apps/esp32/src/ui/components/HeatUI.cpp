#include "ui\components\HeatUI.h"
#include "bitmaps.h"
#include "ui/ColorPalette.h"
#include "ui/base/Screen.h"
#include "ui/components/ZVSOscilloscope.h"

void ZVSOscilloscopeUI(RenderSurface s, ZVSDriver* zvs) {
    static ZVSOscilloscope osc = ZVSOscilloscope(zvs, 280);
    osc.update();
    osc.draw(s, 0, 0, s.height());
}

void Background(RenderSurface s, float progress, uint8_t color) {
    int fillHeight = (int)(s.height() * progress);
    // Rechteck von unten nach oben
    //s.sprite->fillRect(s.left(), s.bottom(), s.width(), s.height(), COLOR_BLACK);
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
}

void Temperature(RenderSurface s, uint8_t  current, uint8_t target) {
    s.sprite->drawBitmap(0, 20, image_temp_48, 48, 48, COLOR_TEXT_PRIMARY);
    s.text(70, 40, isnan(current) ? "Err" : String(current), TextSize::bxl);
    s.text(165, 40, String(target), TextSize::xl);
}


void HeatCycleIndicator(RenderSurface s, uint8_t  cycle) {
    if (cycle == 1) {
        s.sprite->drawBitmap(s.width() - 48, 20, image_cap_fill_48, 48, 48, COLOR_TEXT_PRIMARY);
    }
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



void HeatUI(UI* _ui, HeatState state, ZVSDriver* zvs) {
    _ui->withSurface(280, 205, 0, 35, [&](RenderSurface& s) {
        uint8_t tempColor = 15; //ColorUtils::getTemperatureColor(state.currentTemp);
        s.sprite->setPaletteColor(15, ColorUtils::getTemperatureColor565(state.currentTemp, true));

        Background(s, state.progress, tempColor);
        ZVSOscilloscopeUI(s, zvs);

        Timer(s, state.elapsedSeconds);
        HeatCycleIndicator(s, state.currentCycle);
        Temperature(s, state.currentTemp, state.targetTemp);

        if (DeviceState::instance().zvsDebug) ZVSDebug(s, zvs);
    });
}