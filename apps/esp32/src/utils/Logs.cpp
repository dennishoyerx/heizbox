#include "utils/Logs.h"

namespace Log {
    char* Firmware() {
        char firmwareInfo[64];
        snprintf(firmwareInfo, sizeof(firmwareInfo), "%s (%s)", FIRMWARE_VERSION, BUILD_DATE);
        return firmwareInfo;
    }

}
