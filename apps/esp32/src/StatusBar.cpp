// src/StatusBar.cpp
#include "StatusBar.h"
#include "bitmaps.h"

StatusBar::StatusBar(TFT_eSPI* tft_instance, uint16_t width, ClockManager* cm, uint8_t h)
    : clock(cm), tft(tft_instance), height(h)
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

    const uint32_t now = millis();

    // ========================================================================
    // Check Time (nur alle 5s prüfen)
    // ========================================================================
    if (now - cache.lastUpdate >= TIME_UPDATE_INTERVAL_MS || cache.lastUpdate == 0) {
        String timeStr = clock->getFormattedTime();
        if (timeStr != cache.time) {
            cache.time = timeStr;
            dirty.time = true;
        }
        cache.lastUpdate = now;
    }

    // ========================================================================
    // Check WiFi (nur bei Änderungen)
    // ========================================================================
    const wl_status_t currentStatus = WiFi.status();
    const int8_t currentStrength = getWifiStrength();

    if (currentStatus != cache.wifiStatus || currentStrength != cache.wifiStrength) {
        cache.wifiStatus = currentStatus;
        cache.wifiStrength = currentStrength;
        dirty.wifi = true;
    }

    // ========================================================================
    // Selective Rendering
    // ========================================================================
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
    tft->fillRect(0, 0, 150, height, tft->color565(255, 107, 43));

    // Render Zeit
    tft->setTextColor(TFT_WHITE);
    tft->setFreeFont(&FreeSans12pt7b);
    tft->setTextSize(1);
    tft->setCursor(15, tft->fontHeight() + 4);
    tft->print(cache.time);
}

void StatusBar::drawWifiRegion() {
    // Clear nur WiFi-Icon Bereich
    const int16_t iconX = tft->width() - 40;
    const int16_t iconY = height - 27;
    tft->fillRect(iconX, iconY, 19, 15, tft->color565(255, 107, 43));

    // Render WiFi-Icon
    const uint8_t* icon = getWifiIcon();
    tft->drawBitmap(iconX, iconY, icon, 19, 15, TFT_WHITE);
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
        return image_wifi_not_connected_bits;
    }

    switch (cache.wifiStrength) {
        case 4:  return image_wifi_100_bits;
        case 3:  return image_wifi_75_bits;
        case 2:  return image_wifi_50_bits;
        case 1:  return image_wifi_25_bits;
        default: return image_wifi_0_bits;
    }
}

void StatusBar::pushSprite(int16_t x, int16_t y) {
    // Nicht verwendet - direkt rendering
}
