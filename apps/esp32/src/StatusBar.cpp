#include "StatusBar.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// WiFi symbol bitmap (example, adjust as needed)
static const unsigned char PROGMEM image_wifi_full_bits[] = { 0x01,0xf0,0x00,0x07,0xfc,0x00,0x1e,0x0f,0x00,0x39,0xf3,0x80,0x77,0xfd,0xc0,0xef,0x1e,0xe0,0x5c,0xe7,0x40,0x3b,0xfb,0x80,0x17,0x1d,0x00,0x0e,0xee,0x00,0x05,0xf4,0x00,0x03,0xb8,0x00,0x01,0x50,0x00,0x00,0xe0,0x00,0x00,0x40,0x00 };

static const unsigned char PROGMEM image_wifi_not_connected_bits[] = { 0x21,0xf0,0x00,0x16,0x0c,0x00,0x08,0x03,0x00,0x25,0xf0,0x80,0x42,0x0c,0x40,0x89,0x02,0x20,0x10,0xa1,0x00,0x23,0x58,0x80,0x04,0x24,0x00,0x08,0x52,0x00,0x01,0xa8,0x00,0x02,0x04,0x00,0x00,0x42,0x00,0x00,0xa1,0x00,0x00,0x40,0x80 };

StatusBar::StatusBar(TFT_eSPI* tft_instance, uint16_t width, ClockManager* cm, uint8_t h)
    : clock(cm), height(h), lastMinuteUpdate(0), tft_display(tft_instance) {
}

void StatusBar::draw() {
    if (!clock) return;

    TFT_eSPI* target = tft_display;
    if (!target) return;

    unsigned long currentMillis = millis();

    // Update time only once a minute, or on first draw
    if (currentMillis - lastMinuteUpdate >= 5000 || lastMinuteUpdate == 0) {
        String timeStr = clock->getFormattedTime();
        if (timeStr != lastDisplayedTime) {
            lastMinuteUpdate = currentMillis;
            lastDisplayedTime = timeStr;

            // Clear the time area before redrawing
            target->fillRect(0, 0, 280, height, 0x885);

            // Draw time
            target->setTextColor(TFT_WHITE);
            target->setFreeFont(&FreeSans9pt7b);
            target->setTextSize(1.75);
            target->setCursor(15, target->fontHeight() + 4);
            target->print(timeStr);
        }
    }

    // Draw WiFi symbol
    wl_status_t currentWifiStatus = WiFi.status();
    if (currentWifiStatus == WL_CONNECTED) {
        target->drawBitmap(target->width() - 40, height - 27, image_wifi_full_bits, 19, 15, TFT_WHITE);
    } else {
        target->drawBitmap(target->width() - 40, height - 27, image_wifi_not_connected_bits, 19, 15, TFT_WHITE);
    }
    // Clear WiFi icon area before redrawing only if status changed
    if (currentWifiStatus != lastWifiStatus) {
        lastWifiStatus = currentWifiStatus;
        target->fillRect(target->width() - 40, 0, 19, height, 0x885); // Clear right part of status bar
    }
}

void StatusBar::pushSprite(int16_t x, int16_t y) {
    // No sprite to push, drawing is done directly to tft_display
}
