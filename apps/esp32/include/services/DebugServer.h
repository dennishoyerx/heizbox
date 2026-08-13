#pragma once

#include <WebServer.h>
#include <functional>

class DebugServer {
public:
    static DebugServer& instance() {
        static DebugServer s;
        return s;
    }

    void init();
    void update();
    using UpdateCallback = std::function<bool()>;
    void setUpdateCallback(UpdateCallback cb) { updateCb_ = std::move(cb); }

private:
    WebServer server{80};
    UpdateCallback updateCb_;
    bool otaTooBig_ = false;
    size_t otaReceived_ = 0;

    void handleRoot();
    void handleApiStatus();
    void handleApiLog();
    void handleApiNetTest();
    void handleApiSettingsGet();
    void handleApiSettingsPost();
    void handleApiUpdate();
    void handleApiOtaDone();
    void handleApiOtaUpload();
    void handleNotFound();
};
