#include "driver/net/WebSocketManager.h"

WebSocketManager& WebSocketManager::instance() {
    static WebSocketManager instance;
    return instance;
}

void WebSocketManager::init(const char* url, const char* deviceId, const char* clientType) {
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

    host = host.c_str();
    path = path.c_str();

    Serial.printf("WebSocket connecting to: %s%s\n", host, path);
    
    webSocket.begin(host.c_str(), 80, path.c_str()); 
    //webSocket.beginSSL(host, 443, path, "", "");
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
            state.connected = false;
            if (connectionCallback) connectionCallback(false);
            break;

        case WStype_CONNECTED:
            Serial.printf("WebSocket connected");
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
    WebSocketManager::instance().handleEvent(type, payload, length);
}
