#include "utils/Logs.h"

namespace Log {
    char* Firmware() {
        static char firmwareInfox[64];
        snprintf(firmwareInfox, sizeof(firmwareInfox), "%s (%s)", FIRMWARE_VERSION, BUILD_DATE);
        return firmwareInfox;
    }

}
