#include "utils/clock.h"
#include <time.h>

namespace Utils {
    String getFormattedTime() {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            return "--:--";
        }
        char buffer[6];
        strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
        return String(buffer);
    }

    String getFormattedDate() {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            return "--.--.--";
        }
        char buffer[11];
        strftime(buffer, sizeof(buffer), "%d.%m.%Y", &timeinfo);
        return String(buffer);
    }
}
