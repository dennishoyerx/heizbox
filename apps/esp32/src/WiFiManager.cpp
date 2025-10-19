// src/WiFiManager.cpp
#include "WiFiManager.h"

WiFiManager* WiFiManager::instance = nullptr;

WiFiManager::WiFiManager()
    : connectionCallback(nullptr),
      lastStatusCheck(0)
{
    state.lastStatus = WL_IDLE_STATUS;
    state.lastReconnectAttempt = 0;
    state.reconnectCount = 0;
    state.notifiedConnected = false;

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
            state.reconnectCount = 0;

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

    if (now - state.lastReconnectAttempt >= RECONNECT_INTERVAL_MS) {
        state.reconnectCount++;
        state.lastReconnectAttempt = now;

        Serial.printf("Reconnecting... (attempt %lu)\n", state.reconnectCount);
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
            Serial.printf("Got IP: %s\n", WiFi.localIP().toString().c_str());
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("WiFi disconnected");
            break;
        default:
            break;
    }
}
