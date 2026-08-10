#pragma once
#include <Arduino.h>

class FirmwareUpdater {
public:
    FirmwareUpdater();

    // Einmaliger Check (z.B. nach WiFi-Connect)
    void checkNow();
    // Im Loop aufrufen, prüft Intervall
    void update();
    // Aktuell laufendes Update?
    bool isUpdating() const { return updating; }

private:
    bool checkVersion();
    bool downloadAndFlash(const char* url, size_t size);
    static int parseVersionPart(const String& v, int index);

    uint32_t lastCheck = 0;
    static constexpr uint32_t CHECK_INTERVAL_MS = 6UL * 60 * 60 * 1000; // alle 6h
    bool updating = false;
};
