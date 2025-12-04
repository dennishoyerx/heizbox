#pragma once

#include <memory>
#include "ui/screens/FireScreen.h"
#include "ui/screens/ScreensaverScreen.h"
#include "ui/screens/OtaUpdateScreen.h"
#include "ui/screens/TimezoneScreen.h"
#include "ui/screens/StartupScreen.h"

#include "ui/components/MenuBuilder.h"
#include "base/GenericMenuScreen.h"

#include "heater/HeaterController.h"

class Screens {
public:
    Screens(HeaterController& heater);
    void setup(ScreenManager& screenManager);
    void setupMenus(ScreenManager& screenManager);

private:
    HeaterController& heater;
    std::unique_ptr<FireScreen> fireScreen;
    std::unique_ptr<ScreensaverScreen> screensaverScreen;
    std::unique_ptr<OtaUpdateScreen> otaUpdateScreen;
    std::unique_ptr<TimezoneScreen> timezoneScreen;
    std::unique_ptr<StartupScreen> startupScreen;
    std::unique_ptr<GenericMenuScreen> mainMenuScreen;
    std::unique_ptr<GenericMenuScreen> heaterMenuScreen;
};
