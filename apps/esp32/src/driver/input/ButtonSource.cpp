#include "driver/input/ButtonSource.h"

void Pcf8574ButtonSource::update() {
    // Quick probe: check whether device ACKs — avoids long waits if absent/bus stuck
    Wire.beginTransmission(address);
    uint8_t err = Wire.endTransmission();
    if (err != 0) {
        // device not responding (no ACK) or bus error → don't block, keep previous state
        // optionally set state = 0xFF to indicate "not pressed"
        // state = 0xFF;
        return;
    }

    // Request one byte; check return value and available bytes before read
    uint8_t len = Wire.requestFrom(address, (uint8_t)1);
    if (len == 1 && Wire.available()) {
        int r = Wire.read();
        if (r >= 0) state = (uint8_t)r;
    } else {
        // nothing received — leave state unchanged or set to default
        // state = 0xFF;
    }
}