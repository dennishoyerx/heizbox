#include "Logger.h"

// Helper function for variadic arguments
void vlogPrint(const char* type, const char* format, va_list args) {
    char loc_buf[256];
    char web_buf[256];

    va_list args_copy;
    va_copy(args_copy, args);

    // For Serial output
    int len = vsnprintf(loc_buf, sizeof(loc_buf), format, args);
    if (len > 0) {
        Serial.println(loc_buf);
    }

    // For WebSocket output
    int web_len = vsnprintf(web_buf, sizeof(web_buf), format, args_copy);
    if (web_len > 0) {
        if (WebSocketManager::instance() && WebSocketManager::instance()->isConnected()) {
            String escaped_web_buf = web_buf;
            escaped_web_buf.replace("\n", "\\n"); // Escape newline characters
            String jsonPayload = "{\"t\":\"" + String(type) + "\",\"m\":\"" + escaped_web_buf + "\"}";
            WebSocketManager::instance()->webSocket.sendTXT(jsonPayload);
        }
    }

    va_end(args_copy);
}

// Function to log messages to Serial and WebSocket with explicit type
void logPrint(const char* type, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlogPrint(type, format, args);
    va_end(args);
}

void cflog(const char* type, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlogPrint(type, format, args);
    va_end(args);
}

// Function to log messages to Serial and WebSocket with default type "log"
void logPrint(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlogPrint("log", format, args);
    va_end(args);
}
