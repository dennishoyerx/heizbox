#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "WebSocketManager.h"

// Function to log messages to Serial and WebSocket
void logPrint(const char* format, ...) __attribute__((format(printf, 1, 2)));

#endif // LOGGER_H
