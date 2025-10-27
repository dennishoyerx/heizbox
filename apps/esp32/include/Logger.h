#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "WebSocketManager.h"
#include <stdarg.h>

// Helper function for variadic arguments
void vlogPrint(const char* type, const char* format, va_list args);

// Function to log messages to Serial and WebSocket with explicit type
void logPrint(const char* type, const char* format, ...) __attribute__((format(printf, 2, 3)));

// Function to log messages to Serial and WebSocket with default type "log"
void logPrint(const char* format, ...) __attribute__((format(printf, 1, 2)));

#endif // LOGGER_H
