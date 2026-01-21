#include "ui/components/StatusBar.h"
#include "bitmaps.h"
#include "driver/net/WiFiManager.h"
#include "driver/net/WebSocketManager.h"
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
        state.time = Utils::getFormattedTime();
        state.lastUpdate = now;
    }

    // Check WiFi (nur bei Änderungen)
    state.wifiStatus = WiFi.status();
    state.wifiStrength = getWifiStrength();

    ui->withSurface(96, 50, 0, 190, {
        {"time", state.time},
        {"wifiStatus", state.wifiStatus},
        {"wifiStrength", state.wifiStrength}
    },[this](RenderSurface& s) {
        s.sprite->fillRect(0, 0, s.width(), s.height(), COLOR_BG_2);
        drawTimeRegion(s);
        drawWifiRegion(s);
    });
}

void StatusBar::drawTimeRegion(RenderSurface s) {
    s.text(8, 0, String(state.time), TextSize::lg);
}

void StatusBar::drawWifiRegion(RenderSurface s) {
    // Clear nur WiFi-Icon Bereich
    const int8_t iconSize = 6;
    const int8_t iconRadius = iconSize / 2;
    const int16_t iconX = 16; //16 + s.sprite->textWidth(state.time) + 10;
    const int16_t iconY = height - 6 - iconRadius; // height / 2 - iconRadius;
    s.sprite->fillRect(iconX, iconY, iconSize, iconSize, COLOR_BLACK);

    if (!WebSocketManager::instance().isConnected()) s.sprite->fillCircle(iconX + iconRadius + 15, iconY + iconRadius, iconRadius, COLOR_BLUE);
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

