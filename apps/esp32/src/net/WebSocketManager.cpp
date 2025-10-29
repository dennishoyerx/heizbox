#include "net/WebSocketManager.h"



WebSocketManager* WebSocketManager::instance = nullptr;

WebSocketManager* WebSocketManager::getInstance() {
    return instance;
}

WebSocketManager::WebSocketManager()
    : messageCallback(nullptr),
      connectionCallback(nullptr),
      connected(false)
{
    state.connected = false;
    state.lastHeartbeat = 0;
    state.reconnectAttempts = 0;

    instance = this;
}

void WebSocketManager::init(const char* url, const char* deviceId, const char* clientType) {
    // Parse URL: wss://host/path
    String urlStr(url);

    const char* protocol = "wss://";
    const size_t protocolLen = 6;

    if (urlStr.startsWith(protocol)) {
        urlStr = urlStr.substring(protocolLen);
    }

    int pathIndex = urlStr.indexOf('/');
    String host = urlStr.substring(0, pathIndex);
    String path = urlStr.substring(pathIndex);

    // Append query params
    path += "?deviceId=" + String(deviceId) + "&type=" + String(clientType);

    Serial.printf("WebSocket connecting to: %s%s\n", host.c_str(), path.c_str());

    webSocket.beginSSL(host.c_str(), 443, path.c_str(), "", "/");
    webSocket.onEvent(onWebSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void WebSocketManager::update() {
    webSocket.loop();

    // Auto heartbeat
    if (connected && (millis() - state.lastHeartbeat >= HEARTBEAT_INTERVAL_MS)) {
        sendHeartbeat();
    }
}

// ============================================================================
// Send Methods
// ============================================================================

bool WebSocketManager::sendJson(const JsonDocument& doc) {
    if (!connected) {
        Serial.println("WebSocket not connected");
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
    JsonDocument doc;
    doc["type"] = "heatCycleCompleted";
    doc["duration"] = durationSec;
    doc["cycle"] = cycle;

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
            Serial.println("WebSocket disconnected");
            connected = false;
            if (connectionCallback) connectionCallback(false);
            break;

        case WStype_CONNECTED:
            Serial.printf("WebSocket connected: %s\n", payload);
            connected = true;
            state.reconnectAttempts = 0;
            state.lastHeartbeat = millis();

            // Send initial status
            sendStatusUpdate(true, false);

            if (connectionCallback) connectionCallback(true);
            break;

        case WStype_TEXT:
            {
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload, length);

                if (error) {
                    Serial.printf("JSON parse error: %s\n", error.c_str());
                    return;
                }

                const char* msgType = doc["type"];
                if (msgType && messageCallback) {
                    messageCallback(msgType, doc);
                }
            }
            break;

        case WStype_ERROR:
            Serial.printf("WebSocket error: %s\n", payload);
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
    if (instance) {
        instance->handleEvent(type, payload, length);
    }
}
