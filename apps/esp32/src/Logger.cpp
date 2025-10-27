#include "Logger.h"
#include <stdarg.h>

void logPrint(const char* format, ...) {
    char loc_buf[256];
    char web_buf[256];

    va_list arg;
    va_list arg_copy;

    va_start(arg, format);
    va_copy(arg_copy, arg);

    // For Serial output
    int len = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    if (len > 0) {
        Serial.println(loc_buf);
    }

    // For WebSocket output
    int web_len = vsnprintf(web_buf, sizeof(web_buf), format, arg_copy);
    if (web_len > 0) {
        if (WebSocketManager::getInstance() && WebSocketManager::getInstance()->isConnected()) {
            String jsonPayload = "{\"type\":\"log\",\"message\":\"" + String(web_buf) + "\"}";
            WebSocketManager::getInstance()->webSocket.sendTXT(jsonPayload);
        }
    }

    va_end(arg_copy);
    va_end(arg);
}
