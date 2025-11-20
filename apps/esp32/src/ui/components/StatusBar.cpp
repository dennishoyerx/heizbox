// src/StatusBar.cpp
#include "ui/components/StatusBar.h"
#include "Fonts/GFXFF/gfxfont.h"
#include "bitmaps.h"
#include "net/WiFiManager.h"
#include "utils/clock.h"
#include "ui/ColorPalette.h"

StatusBar::StatusBar(TFT_eSPI* tft_instance, uint16_t width, uint8_t h)
    : tft(tft_instance), height(h)
{
    cache.time = "";
    cache.wifiStatus = WL_IDLE_STATUS;
    cache.wifiStrength = -1;
    cache.lastUpdate = 0;

    dirty.time = true;
    dirty.wifi = true;
}

void StatusBar::draw() {
    if (!clock || !tft) return;
    tft->fillRect(0, height-1, 280, 2, heizbox_palette[COLOR_BG_2]);
    const uint32_t now = millis();

    // Check Time (nur alle 5s prüfen)
    if (now - cache.lastUpdate >= TIME_UPDATE_INTERVAL_MS || cache.lastUpdate == 0) {
        String timeStr = Utils::getFormattedTime();
        if (timeStr != cache.time) {
            cache.time = timeStr;
            dirty.time = true;
        }
        cache.lastUpdate = now;
    }

    // Check WiFi (nur bei Änderungen)
    const wl_status_t currentStatus = WiFi.status();
    const int8_t currentStrength = getWifiStrength();

    if (currentStatus != cache.wifiStatus || currentStrength != cache.wifiStrength) {
        cache.wifiStatus = currentStatus;
        cache.wifiStrength = currentStrength;
        dirty.wifi = true;
    }

    if (dirty.time) {
        drawTimeRegion();
        dirty.time = false;
    }

    if (dirty.wifi) {
        drawWifiRegion();
        dirty.wifi = false;
    }
}

void StatusBar::drawTimeRegion() {
    // Clear nur Zeit-Bereich
    tft->fillRect(280/2 - tft->textWidth(cache.time)/2, 0, tft->textWidth(cache.time), height, heizbox_palette[COLOR_BG]);

    // Render Zeit
    tft->setTextColor(heizbox_palette[COLOR_TEXT_PRIMARY]);
    tft->setFreeFont(&FreeSans12pt7b);
    tft->setTextSize(1);
    tft->setCursor(280/2 - tft->textWidth(cache.time)/2, tft->fontHeight() - 4);
    tft->print(cache.time);
}

void StatusBar::drawWifiRegion() {
    // Clear nur WiFi-Icon Bereich
    const int8_t iconSize = 10;
    const int8_t iconRadius = iconSize / 2;
    const int16_t iconX = (280/2 + tft->textWidth(cache.time)/2) + 10;
    const int16_t iconY = height / 2 - iconRadius;
    tft->fillRect(iconX, iconY, iconSize, iconSize, heizbox_palette[COLOR_BG]);

    if (cache.wifiStatus != WL_CONNECTED) return;

    // Render WiFi-Icon
    const uint8_t* icon = getWifiIcon();
    const uint8_t strength = cache.wifiStrength;

    
    uint8_t color = COLOR_TEXT_PRIMARY;
    if (strength >= 3) {
        color = COLOR_SUCCESS;
    } else if (strength >= 2) {
        color = COLOR_WARNING;
    } else if (strength >= 1) {
        color = COLOR_ERROR;
    }

    tft->fillCircle(iconX + iconRadius, iconY + iconRadius, iconRadius, heizbox_palette[color]);
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

const uint8_t* StatusBar::getWifiIcon() const {
    if (cache.wifiStatus != WL_CONNECTED) {
        return image_wifi_slash_bold_40;
    }

    switch (cache.wifiStrength) {
        case 4:  return image_wifi_high_bold_40;
        case 3:  return image_wifi_medium_bold_40;
        case 2:  return image_wifi_medium_bold_40;
        case 1:  return image_wifi_low_bold_40;
        default: return image_wifi_low_bold_40;
    }
}

void StatusBar::pushSprite(int16_t x, int16_t y) {
    // Nicht verwendet - direkt rendering
}

