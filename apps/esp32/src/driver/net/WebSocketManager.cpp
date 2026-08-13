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

    // Query params anhängen
    pathStr += "?deviceId=" + String(deviceId) + "&type=" + String(clientType);

    // Sichere Kopien in char-Puffer (vermeidet self-assignment-Bug von String.c_str())
    char hostBuf[96];
    char pathBuf[160];
    hostStr.toCharArray(hostBuf, sizeof(hostBuf));
    pathStr.toCharArray(pathBuf, sizeof(pathBuf));

    logPrint("ws", "WebSocket connecting to: %s%s", hostBuf, pathBuf);
    
    // Cloudflare erzwingt TLS: Port 80 gibt 301 -> wss auf 443 nötig.
    // beginSSL mit leerem Fingerprint = kein Zertifikats-Pinning (Cloudflare-TLS ok).
    webSocket.beginSSL(hostBuf, 443, pathBuf, "", "");
    webSocket.onEvent(onWebSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void WebSocketManager::update() {
    webSocket.loop();

    // Auto heartbeat
    if (state.connected && (millis() - state.lastHeartbeat >= HEARTBEAT_INTERVAL_MS)) {
        sendHeartbeat();
    }

    // Manual reconnect
    /*if (!state.connected && (millis() - state.lastReconnectAttempt >= 5000)) {
        Serial.println("Attempting WebSocket reconnect...");
        webSocket.beginSSL(host, 443, path, "", "/");
        webSocket.onEvent(onWebSocketEvent);
        state.lastReconnectAttempt = millis();
    }*/
}

// ============================================================================
// Send Methods
// ============================================================================

bool WebSocketManager::sendJson(const JsonDocument& doc) {
    if (!state.connected) {
        logPrint("ws", "WebSocket not connected");
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

bool WebSocketManager::sendHeartbeat() {
    JsonDocument doc;
    doc["type"] = "heartbeat";
    doc["isOn"] = true;

    state.lastHeartbeat = millis();
    return sendJson(doc);
}

bool WebSocketManager::sendStatusUpdate(bool isOn, bool isHeating) {
    JsonDocument doc;
    doc["type"] = "statusUpdate";
    doc["isOn"] = isOn;
    doc["isHeating"] = isHeating;

    return sendJson(doc);
}

bool WebSocketManager::sendHeatCycleCompleted(uint32_t durationSec, uint8_t cycle) {
    if (!state.connected) {
        Serial.println("WebSocket disconnected, buffering heatCycleCompleted");
        //pendingHeatCycles.push_back({durationSec, cycle});
        return false;
    }

    JsonDocument doc;
    doc["type"] = "heatCycleCompleted";
    doc["duration"] = durationSec;
    doc["cycle"] = cycle;

    return sendJson(doc);
}

bool WebSocketManager::sendTempReading(float tempRaw, float tempCalibrated, bool isHeating) {
    if (!state.connected) return false;

    JsonDocument doc;
    doc["type"] = "tempReading";
    doc["temp_raw"] = tempRaw;
    doc["temp_calibrated"] = tempCalibrated;
    doc["is_heating"] = isHeating;

    return sendJson(doc);
}

bool WebSocketManager::sendSessionUpdate(int clicks, int caps) {
    JsonDocument doc;
    doc["type"] = "sessionUpdate";
    doc["clicks"] = clicks;
    doc["caps"] = caps;

    return sendJson(doc);
}

// ============================================================================
// Event Handling
// ============================================================================

void WebSocketManager::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            logPrint("ws", "WebSocket disconnected");
            state.connected = false;
            if (connectionCallback) connectionCallback(false);
            break;

        case WStype_CONNECTED:
            logPrint("ws", "WebSocket connected");
            state.connected = true;
            state.reconnectAttempts = 0;
            state.lastHeartbeat = millis();

            // Send initial status
            sendStatusUpdate(true, false);

            // pending heatCycleCompleted Messages senden
            /*while (!pendingHeatCycles.empty()) {
                auto msg = pendingHeatCycles.front();
                pendingHeatCycles.pop_front();
                sendHeatCycleCompleted(msg.durationSec, msg.cycle);
            }*/

            if (connectionCallback) connectionCallback(true);
            break;

        case WStype_TEXT:
            {
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload, length);

                if (error) {
                    logPrint("ws", "JSON parse error: %s", error.c_str());
                    return;
                }

                const char* msgType = doc["type"];
                if (msgType && messageCallback) {
                    messageCallback(msgType, doc);
                }
            }
            break;

        case WStype_ERROR:
            if (payload) logPrint("ws", "WebSocket error: %s", payload);
            else logPrint("ws", "WebSocket error (no payload)");
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
