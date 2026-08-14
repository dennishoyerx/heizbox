// src/WiFiManager.cpp
#include "driver/net/WiFiManager.h"

WiFiManager* WiFiManager::instance = nullptr;

WiFiManager::WiFiManager()
    : connectionCallback(nullptr),
      lastStatusCheck(0)
{
    state.lastStatus = WL_IDLE_STATUS;
    state.lastReconnectAttempt = 0;
    state.reconnectCount = 0;
    state.notifiedConnected = false;
    state.maxRetries = 3;  // Max 3 Reconnect-Versuche
    instance = this;
}

void WiFiManager::init(const char* ssid, const char* password, const char* hostname) {
    WiFi.setHostname(hostname);
    WiFi.onEvent(onWiFiEvent);
    WiFi.begin(ssid, password);
    Serial.printf("WiFi connecting to: %s\n", ssid);
}

void WiFiManager::update() {
    const uint32_t now = millis();
    
    // Status check alle 1s
    if (now - lastStatusCheck >= STATUS_CHECK_INTERVAL_MS) {
        checkStatus();
        lastStatusCheck = now;
    }
}

void WiFiManager::checkStatus() {
    const wl_status_t currentStatus = WiFi.status();
    
    // Status-Änderung erkannt
    if (currentStatus != state.lastStatus) {
        state.lastStatus = currentStatus;
        
        if (currentStatus == WL_CONNECTED) {
            // Connected
            if (!state.notifiedConnected && connectionCallback) {
                connectionCallback(true);
                state.notifiedConnected = true;
            }
            state.reconnectCount = 0;  // Reset bei Erfolg
            Serial.printf("WiFi connected: %s (RSSI: %d dBm)\n",
                         WiFi.localIP().toString().c_str(), WiFi.RSSI());
        } else {
            // Disconnected
            if (state.notifiedConnected && connectionCallback) {
                connectionCallback(false);
                state.notifiedConnected = false;
            }
            handleDisconnection();
        }
    }
}

void WiFiManager::handleDisconnection() {
    const uint32_t now = millis();
    
    // Max. 3 Reconnect-Versuche, dann WiFi deaktivieren
    if (state.reconnectCount >= state.maxRetries) {
        Serial.println("WiFi: Max retries reached, disabling WiFi");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);  // WiFi komplett deaktivieren
        return;
    }
    
    if (now - state.lastReconnectAttempt >= RECONNECT_INTERVAL_MS) {
        state.reconnectCount++;
        state.lastReconnectAttempt = now;
        Serial.printf("WiFi reconnecting... (attempt %lu/%d)\n", 
                     state.reconnectCount, state.maxRetries);
        WiFi.reconnect();
    }
}

void WiFiManager::onConnectionChange(ConnectionCallback callback) {
    connectionCallback = callback;
}

void WiFiManager::onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("WiFi started");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("WiFi connected");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.println("Got IP: ");
            if (instance && instance->connectionCallback) {
                instance->connectionCallback(true);
            }
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("WiFi disconnected");
            if (instance && instance->connectionCallback) {
                instance->connectionCallback(false);
            }
            break;
        default:
            break;
    }
}
