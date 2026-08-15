#include "driver/net/WebSocketManager.h"
#include "utils/Logger.h"

WebSocketManager& WebSocketManager::instance() {
    static WebSocketManager instance;
    return instance;
}

void WebSocketManager::init(const char* url, const char* deviceId, const char* clientType) {
    String urlStr(url);
    urlStr.trim();

    // "wss://" oder "ws://" Prefix strippen (plain WS Port 80, kein TLS - RAM-Limit)
    if (urlStr.startsWith("wss://")) {
        urlStr = urlStr.substring(6);
    } else if (urlStr.startsWith("ws://")) {
        urlStr = urlStr.substring(5);
    }

    int pathIndex = urlStr.indexOf('/');
    String hostStr = (pathIndex > 0) ? urlStr.substring(0, pathIndex) : urlStr;
    String pathStr = (pathIndex > 0) ? urlStr.substring(pathIndex) : "/";

    // Query params anh\u00e4ngen
    pathStr += "?deviceId=" + String(deviceId) + "&type=" + String(clientType);

    // Sichere Kopien in char-Puffer (vermeidet self-assignment-Bug von String.c_str())
    char hostBuf[96];
    char pathBuf[160];
    hostStr.toCharArray(hostBuf, sizeof(hostBuf));
    pathStr.toCharArray(pathBuf, sizeof(pathBuf));

    logPrint("ws", "WebSocket connecting to: %s%s", hostBuf, pathBuf);
    
    // Kein TLS: ws:// auf Port 80. Cloudflare redirectet, aber kein Crash.
    webSocket.begin(hostBuf, 80, pathBuf);
    webSocket.onEvent(onWebSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void WebSocketManager::update() {
    webSocket.loop();

    // Puffer leeren - NUR hier im Loop-Kontext (grosser Stack) JSON bauen/senden
    if (state.connected) {
        flushQueue();

        // Auto heartbeat
        if (millis() - state.lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
            sendHeartbeat();
        }
    } else {
        // Nicht connected: Queue verwerfen, keine Stale-Messages ansammeln
        pendingHead = 0;
        pendingCount = 0;
    }
}

// ============================================================================
// Queue (puffert Messages, KEIN JSON/Send im Aufrufer-Kontext!)
// ============================================================================

bool WebSocketManager::queuePush(const WsPendingMsg& msg) {
    if (pendingCount >= PENDING_MAX) {
        logPrint("ws", "WS queue full, dropping msg");
        return false;
    }
    uint8_t idx = (pendingHead + pendingCount) % PENDING_MAX;
    pending[idx] = msg;
    pendingCount++;
    return true;
}

bool WebSocketManager::queuePop(WsPendingMsg& msg) {
    if (pendingCount == 0) return false;
    msg = pending[pendingHead];
    pendingHead = (pendingHead + 1) % PENDING_MAX;
    pendingCount--;
    return true;
}

// ============================================================================
// Send Methods - puffern NUR, senden passiert in update()
// ============================================================================

bool WebSocketManager::sendHeartbeat() {
    WsPendingMsg msg;
    msg.type = WsMsgType::HEARTBEAT;
    return queuePush(msg);
}

bool WebSocketManager::sendStatusUpdate(bool isOn, bool isHeating) {
    WsPendingMsg msg;
    msg.type = WsMsgType::STATUS_UPDATE;
    msg.isOn = isOn;
    msg.isHeating = isHeating;
    return queuePush(msg);
}

bool WebSocketManager::sendHeatCycleCompleted(uint32_t durationSec, uint8_t cycle) {
    WsPendingMsg msg;
    msg.type = WsMsgType::HEAT_CYCLE_COMPLETED;
    msg.durationSec = durationSec;
    msg.cycle = cycle;
    return queuePush(msg);
}

bool WebSocketManager::sendSessionUpdate(int clicks, int caps) {
    WsPendingMsg msg;
    msg.type = WsMsgType::SESSION_UPDATE;
    msg.clicks = clicks;
    msg.caps = caps;
    return queuePush(msg);
}

bool WebSocketManager::sendTempReading(float tempRaw, float tempCalibrated, bool isHeating) {
    WsPendingMsg msg;
    msg.type = WsMsgType::TEMP_READING;
    msg.tempRaw = tempRaw;
    msg.tempCalibrated = tempCalibrated;
    msg.isHeating = isHeating;
    return queuePush(msg);
}

// ============================================================================
// Flush - NUR aus update() aufrufen (Loop-Task, ~8KB Stack)
// ============================================================================

bool WebSocketManager::flushQueue() {
    if (!state.connected) return false;

    WsPendingMsg msg;
    bool any = false;
    while (queuePop(msg)) {
        any = true;
        JsonDocument doc;
        switch (msg.type) {
            case WsMsgType::HEARTBEAT:
                doc["type"] = "heartbeat";
                doc["isOn"] = true;
                break;
            case WsMsgType::STATUS_UPDATE:
                doc["type"] = "statusUpdate";
                doc["isOn"] = msg.isOn;
                doc["isHeating"] = msg.isHeating;
                break;
            case WsMsgType::HEAT_CYCLE_COMPLETED:
                doc["type"] = "heatCycleCompleted";
                doc["duration"] = msg.durationSec;
                doc["cycle"] = msg.cycle;
                break;
            case WsMsgType::SESSION_UPDATE:
                doc["type"] = "sessionUpdate";
                doc["clicks"] = msg.clicks;
                doc["caps"] = msg.caps;
                break;
            case WsMsgType::TEMP_READING:
                doc["type"] = "tempReading";
                doc["tempRaw"] = msg.tempRaw;
                doc["tempCalibrated"] = msg.tempCalibrated;
                doc["isHeating"] = msg.isHeating;
                break;
            default:
                continue;
        }
        sendJson(doc);
    }
    return any;
}

bool WebSocketManager::sendJson(const JsonDocument& doc) {
    if (!state.connected) {
        return false;
    }

    char buffer[256];
    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    if (len >= sizeof(buffer)) {
        Serial.println("JSON too large for buffer");
        return false;
    }

    webSocket.sendTXT(buffer, len);
    return true;
}

void WebSocketManager::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
            Serial.printf("[WS] Connected to server\n");
            state.connected = true;
            state.lastHeartbeat = millis();
            // Kein sendStatusUpdate hier - wird ueber Queue in update() geflusht
            if (connectionCallback) connectionCallback(true);
            break;
        }
        case WStype_DISCONNECTED: {
            Serial.printf("[WS] Disconnected from server\n");
            state.connected = false;
            if (connectionCallback) connectionCallback(false);
            break;
        }
        case WStype_TEXT: {
            if (payload == nullptr || length == 0) break;
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload, length);
            if (err) {
                Serial.printf("[WS] JSON parse error: %s\n", err.c_str());
                break;
            }
            const char* typeStr = doc["type"] | "";
            if (messageCallback && typeStr[0] != '\0') {
                messageCallback(typeStr, doc);
            }
            break;
        }
        case WStype_ERROR:
            Serial.printf("[WS] Error\n");
            break;
        case WStype_PING:
            break;
        case WStype_PONG:
            break;
        default:
            break;
    }
}

void WebSocketManager::onMessage(MessageCallback callback) {
    messageCallback = callback;
}

void WebSocketManager::onConnectionChange(ConnectionCallback callback) {
    connectionCallback = callback;
}

void WebSocketManager::onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    WebSocketManager::instance().handleEvent(type, payload, length);
}
