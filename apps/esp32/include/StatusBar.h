#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <Arduino.h>
#include <WiFi.h>
#include "ClockManager.h"
#include <TFT_eSPI.h>

#define STATUS_BAR_HEIGHT 50

class StatusBar {
private:
    ClockManager* clock;
    uint8_t height;
    unsigned long lastMinuteUpdate;
    wl_status_t lastWifiStatus = WL_IDLE_STATUS;
    String lastDisplayedTime = "";
    TFT_eSPI* tft_display;
    int8_t lastWifiStrength = -1;

public:
    StatusBar(TFT_eSPI* tft_instance, uint16_t width, ClockManager* cm, uint8_t h);
    void draw();
    void pushSprite(int16_t x, int16_t y);
};

#endif // STATUSBAR_H
