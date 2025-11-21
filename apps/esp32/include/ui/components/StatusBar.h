#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "utils/clock.h"
#include "ui/base/SurfaceFactory.h"
#include "ui/base/UI.h"


class StatusBar {
public:
    StatusBar(uint16_t width, uint8_t height);
    ~StatusBar() = default;
    void draw(UI* ui);

private:
    // Dimensions
    uint8_t height;

    // State cache (verhindert unnötige Redraws)
    struct {
        String time;
        wl_status_t wifiStatus;
        int8_t wifiStrength;
        uint32_t lastUpdate;
    } state;

    // Dirty flags für selective rendering
    struct {
        bool time : 1;
        bool wifi : 1;
    } dirty;

    // Helper methods
    void drawTimeRegion(RenderSurface s);
    void drawWifiRegion(RenderSurface s);
    int8_t getWifiStrength() const;

    static constexpr uint32_t TIME_UPDATE_INTERVAL_MS = 5000;
    static constexpr uint16_t BG_COLOR = 1;
};
