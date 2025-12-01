// include/hardware/OTASetup.h
#pragma once

#include <ArduinoOTA.h>
#include "utils/Logger.h"
#include "ui/base/ScreenManager.h"
#include "ui/screens/OtaUpdateScreen.h"
#include "ui/screens/FireScreen.h"
#include "ui/DeviceUI.h"

/**
 * @brief Manages the setup and handling of Over-The-Air (OTA) updates.
 *
 * This class encapsulates the configuration of ArduinoOTA, including hostname,
 * and callbacks for start, end, and error events during an OTA update process.
 */
class OTASetup {
public:
    /**
     * @brief Constructs an OTASetup instance.
     * @param screenManager Reference to the ScreenManager instance.
     * @param otaUpdateScreen Reference to the OtaUpdateScreen instance.
     * @param fireScreen Reference to the FireScreen instance.
     */
    OTASetup(DeviceUI& ui);

    /**
     * @brief Sets up ArduinoOTA with the specified hostname and event callbacks.
     */
    void setupOTA();

    /**
     * @brief Handles ArduinoOTA events. Should be called in the main loop.
     */
    void handleOTA();

private:
    DeviceUI& ui;
};
