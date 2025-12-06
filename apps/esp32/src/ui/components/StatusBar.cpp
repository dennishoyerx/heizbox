#include "ui/components/StatusBar.h"
#include "Fonts/GFXFF/gfxfont.h"
#include "bitmaps.h"
#include "net/WiFiManager.h"
#include "utils/clock.h"
#include "ui/ColorPalette.h"

StatusBar::StatusBar(uint16_t width, uint8_t h)
    : height(h)
{
    state.time = "";
    state.wifiStatus = WL_IDLE_STATUS;
    state.wifiStrength = -1;
    state.lastUpdate = 0;

    dirty.time = true;
    dirty.wifi = true;
}

void StatusBar::draw(UI* ui) {  
    if (!clock) return;
    
    const uint32_t now = millis();

    // Check Time (nur alle 5s prüfen)
    if (now - state.lastUpdate >= TIME_UPDATE_INTERVAL_MS || state.lastUpdate == 0) {
        String timeStr = Utils::getFormattedTime();
        if (timeStr != state.time) {
            state.time = timeStr;
            dirty.time = true;
        }
        state.lastUpdate = now;
    }

    // Check WiFi (nur bei Änderungen)
    const wl_status_t currentStatus = WiFi.status();
    const int8_t currentStrength = getWifiStrength();

    if (currentStatus != state.wifiStatus || currentStrength != state.wifiStrength) {
        state.wifiStatus = currentStatus;
        state.wifiStrength = currentStrength;
        dirty.wifi = true;
    }

    ui->withSurface(280, 35, 0, 0, {
        {"time", state.time},
        {"wifiStatus", state.wifiStatus},
        {"wifiStrength", state.wifiStrength}
    },[this](RenderSurface& s) {
        s.sprite->fillRect(0, 0, s.width(), s.height(), COLOR_BLACK);
        drawTimeRegion(s);
        drawWifiRegion(s);
    });
}

void StatusBar::drawTimeRegion(RenderSurface s) {
    // Clear nur Zeit-Bereich
    s.sprite->fillRect(280/2 - s.sprite->textWidth(state.time)/2, 0, s.sprite->textWidth(state.time), height, COLOR_BLACK);

    // Render Zeit
    s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
    s.sprite->setFreeFont(&FreeSans12pt7b);
    s.sprite->setTextSize(1);
    s.sprite->setCursor(280/2 - s.sprite->textWidth(state.time)/2, s.sprite->fontHeight() - 4);
    s.sprite->print(state.time);
}

void StatusBar::drawWifiRegion(RenderSurface s) {
    // Clear nur WiFi-Icon Bereich
    const int8_t iconSize = 10;
    const int8_t iconRadius = iconSize / 2;
    const int16_t iconX = (280/2 + s.sprite->textWidth(state.time)/2) + 10;
    const int16_t iconY = height / 2 - iconRadius;
    s.sprite->fillRect(iconX, iconY, iconSize, iconSize, COLOR_BLACK);

    if (state.wifiStatus != WL_CONNECTED) return;

    // Render WiFi-Icon
    uint8_t color = ColorUtils::getWifiColor(state.wifiStrength);
    s.sprite->fillCircle(iconX + iconRadius, iconY + iconRadius, iconRadius, color);
}

int8_t StatusBar::getWifiStrength() const {
    if (WiFi.status() != WL_CONNECTED) return -1;

    const long rssi = WiFi.RSSI();
    if (rssi >= -55) return 4;       // Excellent
    if (rssi >= -65) return 3;       // Good
    if (rssi >= -75) return 2;       // Fair
    if (rssi >= -85) return 1;       // Weak
    return 0;                        // Very weak
}

