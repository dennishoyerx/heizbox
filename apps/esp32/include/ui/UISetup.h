// include/ui/UISetup.h
#pragma once

#include <memory>
#include "ui/base/ScreenManager.h"
#include "ui/screens/FireScreen.h"
#include "ui/screens/ScreensaverScreen.h"
#include "ui/screens/OtaUpdateScreen.h"
#include "ui/screens/TimezoneScreen.h"
#include "ui/screens/StartupScreen.h"
#include "ui/base/ScreenTransition.h"
#include "ui/components/MenuBuilder.h"
#include "core/StateManager.h"
#include "base/GenericMenuScreen.h"
#include "hardware/sensor/TempSensor.h" // Include TempSensor

/**
 * @brief Manages the setup and registration of all UI screens and the main menu.
 *
 * This class encapsulates the logic for initializing various screens and configuring
 * the main menu with its interactive elements and state bindings.
 */
class UISetup {
public:
    /**
     * @brief Constructs a UISetup instance.
     * @param screenManager Reference to the ScreenManager instance.
     * @param fireScreen Reference to the FireScreen instance.
     * @param screensaverScreen Reference to the ScreensaverScreen instance.
     * @param otaUpdateScreen Reference to the OtaUpdateScreen instance.
     * @param timezoneScreen Reference to the TimezoneScreen instance.
     * @param startupScreen Reference to the StartupScreen instance.
     * @param setCurrentCycleCallback Callback to set the current heat cycle.
     */
    UISetup(
        ScreenManager& screenManager,
        HeaterController& heater,
        DisplayDriver* displayDriver,
        InputManager& inputManager
    );

    void setupScreens();

    FireScreen* getFireScreen() const { return fireScreen.get(); }
    ScreensaverScreen* getScreensaverScreen() const { return screensaverScreen.get(); }
    OtaUpdateScreen* getOtaUpdateScreen() const { return otaUpdateScreen.get(); }
    StartupScreen* getStartupScreen() const { return startupScreen.get(); }
    TimezoneScreen* getTimezoneScreen() const { return timezoneScreen.get(); }

    void setupMainMenu();

private:
    ScreenManager& screenManager;
    HeaterController& heater;
    DisplayDriver* displayDriver;
    InputManager& inputManager;

    std::unique_ptr<GenericMenuScreen> mainMenuScreen;
    std::unique_ptr<GenericMenuScreen> heaterMenuScreen;

    std::unique_ptr<FireScreen> fireScreen;
    std::unique_ptr<ScreensaverScreen> screensaverScreen;
    std::unique_ptr<OtaUpdateScreen> otaUpdateScreen;
    std::unique_ptr<TimezoneScreen> timezoneScreen;
    std::unique_ptr<StartupScreen> startupScreen;
};
