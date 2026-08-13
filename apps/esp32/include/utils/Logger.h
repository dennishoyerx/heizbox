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

// Ringbuffer: speichert die letzten Log-Zeilen für den Debug-Server.
// Thread-sicher via Interrupt-Sperre (Logger wird aus mehreren Tasks aufgerufen).
void logRingPush(const char* type, const char* line);
// Gibt die Zeilen als JSON-Array zurück: {"since":<ts>,"lines":[{"ts":<ms>,"t":"type","m":"msg"}]}
// since = Millis-Timestamp des zuletzt gelieferten Logs (0 = alle).
String logRingJson(uint32_t since);

#endif // LOGGER_H
