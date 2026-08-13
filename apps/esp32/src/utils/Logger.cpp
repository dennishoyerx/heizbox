#include "Logger.h"

size_t RingStream::write(uint8_t c) {
    Serial.write(c);
    if (c == '\n') {
        if (bufLen > 0) {
            buf[bufLen] = '\0';
            logRingPush("lib", buf);
            bufLen = 0;
        }
    } else if (bufLen < sizeof(buf) - 1) {
        buf[bufLen++] = c;
    }
    return 1;
}

RingStream g_ringStream;

// ---- Ringbuffer ----
struct LogEntry {
    uint32_t ts;
    char type[16];
    char msg[LOG_LINE_MAX];
};

static LogEntry logRing[LOG_RING_SIZE];
static volatile uint32_t logRingHead = 0; // nächster freier Slot
static volatile uint32_t logRingCount = 0;

void logRingPush(const char* type, const char* line) {
    uint32_t idx;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    idx = logRingHead % LOG_RING_SIZE;
    logRing[idx].ts = millis();
    strncpy(logRing[idx].type, type, sizeof(logRing[idx].type) - 1);
    logRing[idx].type[sizeof(logRing[idx].type) - 1] = '\0';
    strncpy(logRing[idx].msg, line, sizeof(logRing[idx].msg) - 1);
    logRing[idx].msg[sizeof(logRing[idx].msg) - 1] = '\0';
    logRingHead++;
    if (logRingCount < LOG_RING_SIZE) logRingCount++;
    portEXIT_CRITICAL(&mux);
}

String logRingJson(uint32_t since) {
    String json = "{\"since\":";
    uint32_t head = logRingHead;
    uint32_t count = logRingCount;
    uint32_t start = (count == LOG_RING_SIZE) ? head : 0;
    json += String(head);
    json += ",\"lines\":[";

    bool first = true;
    uint32_t n = (count < LOG_RING_SIZE) ? count : LOG_RING_SIZE;
    for (uint32_t i = 0; i < n; i++) {
        uint32_t idx = (start + i) % LOG_RING_SIZE;
        const LogEntry& e = logRing[idx];
        if (since > 0 && e.ts <= since) continue;
        if (!first) json += ",";
        first = false;
        json += "{\"ts\":" + String(e.ts) + ",\"t\":\"";
        json += e.type;
        json += "\",\"m\":\"";
        String msg = e.msg;
        msg.replace("\\", "\\\\");
        msg.replace("\"", "\\\"");
        msg.replace("\n", "\\n");
        msg.replace("\r", "");
        json += msg;
        json += "\"}";
    }
    json += "]}";
    return json;
}

// ---- Logging ----
// Helper function for variadic arguments
void vlogPrint(const char* type, const char* format, va_list args) {
    WebSocketManager& ws = WebSocketManager::instance();

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
        // In Ringbuffer für Debug-Server
        logRingPush(type, web_buf);

        if (ws.isConnected()) {
            String escaped_web_buf = web_buf;
            escaped_web_buf.replace("\n", "\\n"); // Escape newline characters
            String jsonPayload = "{\"t\":\"" + String(type) + "\",\"m\":\"" + escaped_web_buf + "\"}";
            ws.webSocket.sendTXT(jsonPayload);
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
