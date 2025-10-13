#include "FireScreen.h"
#include "ScreenManager.h"
#include "StatusBar.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>


const unsigned long SCREENSAVER_TIMEOUT = 30000; // 30 seconds

ScreenType FireScreen::getType() const {
    return ScreenType::FIRE;
}

FireScreen::FireScreen(HeaterController& hc, ScreenManager* sm, ScreensaverScreen* ss, std::function<void(int)> setCycleCb)

    : heater(hc), screenManager(sm), screensaverScreen(ss), startTime(0), elapsedTime(0), lastActivityTime(0),

      _currentCycle(1), _showSavedConfirmation(false), _savedConfirmationTime(0),

      _setCycleCallback(setCycleCb) {

}



const unsigned long CONFIRMATION_DISPLAY_TIME = 2000; // 2 seconds



void FireScreen::draw(DisplayManager& display) {

    display.clear(0x885);



    // Display heating status and timer

    //display.drawText(20, 30, "HEATING MODE", ST77XX_WHITE, 2);



    if (heater.isHeating()) {

        unsigned long seconds = elapsedTime / 1000;

        unsigned long minutes = seconds / 60;

        seconds %= 60;



        char timeStr[10];

        sprintf(timeStr, "%02lu", seconds);

        display.drawText(90, 80, timeStr, ST77XX_WHITE, 6);

    }



    // Display status

    const char* status;

    switch (heater.getState()) {

    case HEATING: status = ""; break;

    case COOLDOWN: status = "COOLDOWN"; break;

    case ERROR: status = "ERROR"; break;

    default: status = ""; break;

    }

    display.drawText(70, 180, status, ST77XX_WHITE, 2);



    // Display current cycle

    char cycleText[15];

    sprintf(cycleText, "Cycle: %d", _currentCycle);

    display.drawText(10, 10, cycleText, ST77XX_WHITE, 2);

}



void FireScreen::update() {

    static bool wasActive = false;

    bool isActive = heater.isHeating() || heater.getState() == COOLDOWN;



    if (isActive) {

        elapsedTime = millis() - startTime;

        lastActivityTime = millis(); // Reset inactivity timer while heating

        static unsigned long lastDirty = 0;

        unsigned long now = millis();

        if (now - lastDirty >= 1000) { // 1 second interval

            screenManager->setDirty();

            lastDirty = now;

        }

    } else if (wasActive) {

        // Condition just changed from true to false, call setDirty once

        screenManager->setDirty();

        elapsedTime = 0;

    }

    wasActive = isActive;



    if (!isActive && (millis() - lastActivityTime > SCREENSAVER_TIMEOUT)) {

        screenManager->setScreen(screensaverScreen);

    }

}



void FireScreen::handleInput(InputEvent event) {

    lastActivityTime = millis();



    if (event.type == PRESS) {

        if (event.button == FIRE && !heater.isHeating()) {

            heater.startHeating();

            startTime = millis();

            elapsedTime = 0;

            screenManager->setDirty();

        } else if (event.button == FIRE && heater.isHeating()) {

            heater.stopHeating();

            screenManager->setDirty();

        } else if (event.button == UP) {

            if (_currentCycle == 1) {

                _currentCycle = 2;

            } else if (_currentCycle == 2) {

                _currentCycle = 1;

            }

            _setCycleCallback(_currentCycle);

            screenManager->setDirty();

        } else if (event.button == DOWN) {

            if (_currentCycle == 1) {

                _currentCycle = 3;

            } else if (_currentCycle == 3) {

                _currentCycle = 4;

            }

            else if (_currentCycle == 4) {

                _currentCycle = 1;

            }

            _setCycleCallback(_currentCycle);

            screenManager->setDirty();

        }

    }

}

void FireScreen::resetActivityTimer() {
    lastActivityTime = millis();
}