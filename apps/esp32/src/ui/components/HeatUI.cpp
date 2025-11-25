#include "ui\components\HeatUI.h"
#include "bitmaps.h"
#include "ui/ColorPalette.h"
#include "ui/base/Screen.h"

class ZVSOscilloscope {
public:
    ZVSOscilloscope(ZVSDriver* zvs, int displayWidth = 280)
        : zvs(zvs), width(displayWidth) {
        buffer.reserve(width);
    }
    
    void update() {
        // Sample current state
        uint8_t value = zvs->isPhysicallyOn() ? 255 : 0;
        buffer.push_back(value);
        
        // Keep only last N samples
        if (buffer.size() > width) {
            buffer.erase(buffer.begin());
        }
    }
    
    void draw(RenderSurface& s, int x, int y, int h) {
        // Draw waveform
        for (size_t i = 1; i < buffer.size(); i++) {
            int x1 = x + i - 1;
            int x2 = x + i;
            int y1 = y + h - (buffer[i-1] * h / 255);
            int y2 = y + h - (buffer[i] * h / 255);
            
            s.sprite->drawLine(x1, y1, x2, y2, COLOR_SUCCESS);
        }
        
        // Draw baseline
        s.sprite->drawLine(x, y + h, x + width, y + h, COLOR_TEXT_SECONDARY);
    }
    
    void reset() {
        buffer.clear();
    }

private:
    ZVSDriver* zvs;
    std::vector<uint8_t> buffer;
    int width;
};


void HeatUI(RenderSurface s, HeatState state, ZVSDriver* zvs) {
    static ZVSOscilloscope osc = ZVSOscilloscope(zvs, 280);
    osc.update();

    uint8_t tempColor = ColorUtils::getTemperatureColor(state.currentTemp);

    int width = s.width();
    int height = s.height();

    int centerX = width / 2;
    int centerY = height / 2;
    int bottomY = centerY + height / 2;
    int leftX = centerX - width / 2;

    // progress = 0.0 .. 1.0
    int fillHeight = (int)(s.height() * state.progress);

    // Rechteck von unten nach oben
    s.sprite->fillRect(leftX, bottomY, width, s.height(), COLOR_BLACK);
    s.sprite->fillRect(leftX, bottomY - fillHeight, width, fillHeight, tempColor);
    osc.draw(s, 0, 0, s.height());

    // === TIMER ===
    char timeStr[4];
    snprintf(timeStr, sizeof(timeStr), "%lu", state.elapsedSeconds);

    s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
    s.sprite->setTextDatum(MC_DATUM);
    s.sprite->setTextSize(2);
    s.sprite->setFreeFont(&FreeSansBold18pt7b);
    s.sprite->drawString(timeStr, centerX, centerY, 1);

    if (state.currentCycle == 1) {
        s.sprite->drawBitmap(width - 48, 20, image_cap_fill_48, 48, 48, COLOR_TEXT_PRIMARY);
    }

    s.sprite->drawBitmap(0, 20, image_temp_48, 48, 48, COLOR_TEXT_PRIMARY);
    s.text(70, 40, isnan(state.currentTemp) ? "Err" : String(state.currentTemp), TextSize::bxl);
    s.text(165, 40, String(state.targetTemp), TextSize::xl);
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