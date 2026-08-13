#pragma once

#include <WebServer.h>

class DebugServer {
public:
    static DebugServer& instance() {
        static DebugServer s;
        return s;
    }

    void init();
    void update();

private:
    WebServer server{80};

    void handleRoot();
    void handleApiStatus();
    void handleApiLog();
    void handleApiOtaDone();
    void handleApiOtaUpload();
    void handleNotFound();
};
