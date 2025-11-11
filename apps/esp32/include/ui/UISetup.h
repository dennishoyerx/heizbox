// include/ui/UISetup.h
#pragma once

#include <memory>
#include "ui/base/ScreenManager.h"
#include "ui/screens/FireScreen.h"
#include "ui/screens/HiddenModeScreen.h"
#include "ui/screens/ScreensaverScreen.h"
#include "ui/screens/OtaUpdateScreen.h"
#include "ui/screens/StatsScreen.h"
#include "ui/screens/TimezoneScreen.h"
#include "ui/screens/StartupScreen.h"
#include "ui/base/ScreenTransition.h"
#include "ui/components/MenuBuilder.h"
#include "core/StateManager.h"
#include "base/GenericMenuScreen.h"

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
     * @param hiddenModeScreen Reference to the HiddenModeScreen instance.
     * @param screensaverScreen Reference to the ScreensaverScreen instance.
     * @param otaUpdateScreen Reference to the OtaUpdateScreen instance.
     * @param statsScreen Reference to the StatsScreen instance.
     * @param timezoneScreen Reference to the TimezoneScreen instance.
     * @param startupScreen Reference to the StartupScreen instance.
     * @param setCurrentCycleCallback Callback to set the current heat cycle.
     */
    UISetup(
        ScreenManager& screenManager,
        FireScreen& fireScreen,
        HiddenModeScreen& hiddenModeScreen,
        ScreensaverScreen& screensaverScreen,
        OtaUpdateScreen& otaUpdateScreen,
        StatsScreen& statsScreen,
        TimezoneScreen& timezoneScreen,
        StartupScreen& startupScreen,
        std::function<void(int)> setCurrentCycleCallback
    );

    /**
     * @brief Sets up and registers all screens with the ScreenManager.
     */
    void setupScreenRegistry();

    void setup();
    void loop();


    /**
     * @brief Sets up the main menu, including its items and actions.
     * @return A unique_ptr to the created GenericMenuScreen.
     */
    std::unique_ptr<GenericMenuScreen> setupMainMenu();

private:
    ScreenManager& screenManager;
    FireScreen& fireScreen;
    HiddenModeScreen& hiddenModeScreen;
    ScreensaverScreen& screensaverScreen;
    OtaUpdateScreen& otaUpdateScreen;
    StatsScreen& statsScreen;
    TimezoneScreen& timezoneScreen;
    StartupScreen& startupScreen;
    std::function<void(int)> setCurrentCycleCallback;
};
