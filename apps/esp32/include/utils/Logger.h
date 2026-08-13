#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "driver/net/WebSocketManager.h"
#include <stdarg.h>

// Ringbuffer-Größe für Debug-Server
#define LOG_RING_SIZE 200
#define LOG_LINE_MAX 192

// Helper function for variadic arguments
void vlogPrint(const char* type, const char* format, va_list args);

// Function to log messages to Serial and WebSocket with explicit type
void logPrint(const char* type, const char* format, ...) __attribute__((format(printf, 2, 3)));

// Function to log messages to Serial and WebSocket with default type "log"
void logPrint(const char* format, ...);

void logRingPush(const char* type, const char* line);

// Stream der Serial-Ausgaben abfängt und in den Ringbuffer spiegelt (für Lib-Debug-Logs)
class RingStream : public Print {
public:
    size_t write(uint8_t c) override {
        Serial.write(c);
        if (c == '\n') {
            if (bufLen > 0) {
                buf[bufLen] = '\0';
                logRingPush("lib", buf);
                bufLen = 0;
            }
        } else if (bufLen < LOG_LINE_MAX - 1) {
            buf[bufLen++] = c;
        }
        return 1;
    }
private:
    char buf[LOG_LINE_MAX];
    size_t bufLen = 0;
};

// Ringbuffer: speichert die letzten Log-Zeilen für den Debug-Server.
// Gibt die Zeilen als JSON-Array zurück: {"since":<ts>,"lines":[{"ts":<ms>,"t":"type","m":"msg"}]}
// since = Millis-Timestamp des zuletzt gelieferten Logs (0 = alle).
String logRingJson(uint32_t since);

#endif // LOGGER_H
