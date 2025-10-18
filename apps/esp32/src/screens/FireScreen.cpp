#include "FireScreen.h"
#include "ScreenManager.h"
#include "StatusBar.h"
#include <Adafruit_GFX.h>
#include <TFT_eSPI.h>

namespace {
    constexpr unsigned long SCREENSAVER_TIMEOUT = 30000; // 30 seconds
    constexpr unsigned long CONFIRMATION_DISPLAY_TIME = 2000; // 2 seconds
}

ScreenType FireScreen::getType() const {
    return ScreenType::FIRE;
}

FireScreen::FireScreen(
    HeaterController& hc,
    ScreenManager* sm,
    ScreensaverScreen* ss,
    StatsManager* stm, // Add StatsManager pointer to constructor
    std::function<void(int)> setCycleCb
)
    : heater(hc),
      screenManager(sm),
      screensaverScreen(ss),
      statsManager(stm), // Initialize StatsManager
      startTime(0),
      elapsedTime(0),
      lastActivityTime(0),
      _currentCycle(1),
      _showSavedConfirmation(false),
      _savedConfirmationTime(0),
      _setCycleCallback(std::move(setCycleCb))
{}

void FireScreen::draw(DisplayManager& display) {
    display.clear(0x885);

    // Display heating timer if heating
    if (heater.isHeating()) {
        unsigned long seconds = elapsedTime / 1000;
        char timeStr[10];
        snprintf(timeStr, sizeof(timeStr), "%02lu", seconds % 60);
        display.drawText(90, 80, timeStr, TFT_WHITE, 6);
    }

    // Display status
    const char* status = "";
    switch (heater.getState()) {
        case HEATING:   status = "";         break;
        case COOLDOWN:  status = "COOLDOWN"; break;
        case ERROR:     status = "ERROR";    break;
        default:        status = "";         break;
    }
    display.drawText(70, 180, status, TFT_WHITE, 2);

    // Display current cycle
    char cycleText[15];
    snprintf(cycleText, sizeof(cycleText), "Cycle: %d", _currentCycle);
    display.drawText(10, 10, cycleText, TFT_WHITE, 3);

    // Display session stats
    char statsText[50];
    snprintf(statsText, sizeof(statsText), "Clicks: %d | Caps: %d", statsManager->getClicks(), statsManager->getCaps());
    display.drawText(10, 40, statsText, TFT_WHITE, 2);

    String consumption = statsManager->getConsumption();
    snprintf(statsText, sizeof(statsText), "Verbrauch: %sg", consumption.c_str());
    display.drawText(10, 170, statsText, TFT_WHITE, 2);
}

void FireScreen::update() {
    static bool wasActive = false;
    bool isActive = heater.isHeating() || heater.getState() == COOLDOWN;
    unsigned long now = millis();

    if (isActive) {
        elapsedTime = now - startTime;
        lastActivityTime = now; // Reset inactivity timer while heating

        static unsigned long lastDirty = 0;
        if (now - lastDirty >= 1000) { // 1 second interval
            screenManager->setDirty();
            lastDirty = now;
        }
    } else if (wasActive) {
        screenManager->setDirty();
        elapsedTime = 0;
    }
    wasActive = isActive;

    if (!isActive && (now - lastActivityTime > SCREENSAVER_TIMEOUT)) {
        screenManager->setScreen(screensaverScreen);
    }
}

void FireScreen::handleInput(InputEvent event) {
    lastActivityTime = millis();

    if (event.type != PRESS) return;

    switch (event.button) {
        case FIRE:
            if (!heater.isHeating()) {
                heater.startHeating();
                startTime = millis();
                elapsedTime = 0;
            } else {
                bool updateCycle = heater.isHeating();
                heater.stopHeating();
                if (updateCycle) {
                    _setCycleCallback(_currentCycle);
                    _currentCycle = (_currentCycle == 1) ? 2 : 1;
				}
            }
            screenManager->setDirty();
            break;

        case UP:
            if (_currentCycle == 1) {
                _currentCycle = 2;
            }
            else if (_currentCycle == 2) {
                _currentCycle = 3;
            }
            else if (_currentCycle == 3) {
                _currentCycle = 4;
            }
            else if (_currentCycle == 4) {
                _currentCycle = 1;
            }
            _setCycleCallback(_currentCycle);
            screenManager->setDirty();
            break;

        case DOWN:
            if (_currentCycle == 1) {
                _currentCycle = 3;
            } else if (_currentCycle == 2) {
                _currentCycle = 1;
            }  else if (_currentCycle == 3) {
                _currentCycle = 4;
            } else if (_currentCycle == 4) {
                _currentCycle = 1;
            }
            _setCycleCallback(_currentCycle);
            screenManager->setDirty();
            break;

        default:
            break;
    }
}

void FireScreen::resetActivityTimer() {
    lastActivityTime = millis();
}
