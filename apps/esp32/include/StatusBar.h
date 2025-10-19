// include/StatusBar.h
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "ClockManager.h"

class StatusBar {
public:
    StatusBar(TFT_eSPI* tft, uint16_t width, ClockManager* cm, uint8_t height);
    ~StatusBar() = default;

    void draw();
    void pushSprite(int16_t x, int16_t y);

private:
    // Components
    ClockManager* clock;
    TFT_eSPI* tft;

    // Dimensions
    uint8_t height;

    // State cache (verhindert unnötige Redraws)
    struct {
        String time;
        wl_status_t wifiStatus;
        int8_t wifiStrength;
        uint32_t lastUpdate;
    } cache;

    // Dirty flags für selective rendering
    struct {
        bool time : 1;
        bool wifi : 1;
    } dirty;

    // Helper methods
    void drawTimeRegion();
    void drawWifiRegion();
    int8_t getWifiStrength() const;
    const uint8_t* getWifiIcon() const;

    static constexpr uint32_t TIME_UPDATE_INTERVAL_MS = 5000;
    static constexpr uint16_t BG_COLOR = 0x885;
};

#endif