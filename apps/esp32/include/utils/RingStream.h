#pragma once

#ifdef __cplusplus

#include <Arduino.h>

// Stream der Serial-Ausgaben abfängt und in den Debug-Log-Ringbuffer spiegelt.
// Wird via build_flag `-include` in JEDER Übersetzungseinheit eingefügt (auch Libs),
// damit DEBUG_ESP_PORT der WebSockets-Lib hierauf zeigen kann.
class RingStream : public Print {
public:
    size_t write(uint8_t c) override;
private:
    char buf[192];
    size_t bufLen = 0;
};

extern RingStream g_ringStream;

#endif // __cplusplus
