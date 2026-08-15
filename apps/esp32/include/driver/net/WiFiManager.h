#pragma once
#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
    using ConnectionCallback = std::function<void(bool)>;

    WiFiManager();
    void init(const char* ssid, const char* password, const char* hostname);
    void update();
    void onConnectionChange(ConnectionCallback callback);
    bool isConnected() const;

private:
    static void onWiFiEvent(WiFiEvent_t event);
    void checkStatus();
    void handleDisconnection();

    ConnectionCallback connectionCallback;
    uint32_t lastStatusCheck;
    
    static constexpr uint32_t STATUS_CHECK_INTERVAL_MS = 1000;
    static constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;  // 5s zwischen Reconnects
    static constexpr uint32_t MAX_RECONNECT_RETRIES = 3;     // Max 3 Versuche

    struct {
        wl_status_t lastStatus;
        uint32_t lastReconnectAttempt;
        uint32_t reconnectCount;
        bool notifiedConnected;
        bool reconnectEnabled;  // false = Reconnect-Loop gestoppt
    } state;

    static WiFiManager* instance;
};
