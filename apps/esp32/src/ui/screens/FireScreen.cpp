#include "ui/screens/FireScreen.h"
#include "ui/base/ScreenManager.h"
#include "ui/components/UIText.h"
#include "bitmaps.h"
#include <TFT_eSPI.h>
#include "utils/Logger.h"
#include "StateManager.h"
#include <utility>

FireScreen::FireScreen(HeaterController& hc, ScreenManager* sm,
                       ScreensaverScreen* ss, StatsManager* stm,
                       std::function<void(int)> setCycleCb)
    : heater(hc),
      screenManager(sm),
      screensaverScreen(ss),
      statsManager(stm),
      setCycleCallback(std::move(setCycleCb)),
      cachedClicks(0),
      cachedConsumption(0),
      cachedTodayConsumption(0),
      cachedCaps(0)
{
    // State-Listener registrieren
    DeviceState::instance().sessionClicks.addListener([this](int clicks) {
        cachedClicks = clicks;
        markDirty();
    });

    DeviceState::instance().sessionCaps.addListener([this](int caps) {
        cachedCaps = caps;
        markDirty();
    });

    DeviceState::instance().sessionConsumption.addListener([this](float consumption) {
        cachedConsumption = consumption;
        markDirty();
    });

    DeviceState::instance().todayConsumption.addListener([this](float consumption) {
        this->cachedTodayConsumption = consumption;
        markDirty();
    });

    state.heatingStartTime = 0;
    state.lastActivityTime = millis();
    state.currentCycle = 1;
    state.showingSavedConfirmation = false;
    state.confirmationStartTime = 0;
}

void FireScreen::onEnter() {
    resetActivityTimer();
}

void FireScreen::draw(DisplayDriver& display) {
    display.clear();  // Dark gray background

    drawHeatingTimer(display);
    drawStatus(display);
    drawCycleInfo(display);
    drawSessionStats(display);

    display.drawText(10, 50, state.touchActive ? "X" : "_", TFT_WHITE, 1);
    char touchValStr[10];
    snprintf(touchValStr, sizeof(touchValStr), "Val: %d", state.currentTouchValue);
    display.drawText(80, 50, touchValStr, TFT_WHITE, 1);
}

void FireScreen::drawHeatingTimer(DisplayDriver& display) {
    if (!heater.isHeating()) return;

    const uint32_t elapsed = millis() - state.heatingStartTime;
    const uint32_t seconds = elapsed / 1000;

    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%02lu", seconds % 60);

    // Zentrierter großer Timer
    centerText(display, 100, timeStr, TFT_WHITE, 5);
}

void FireScreen::drawStatus(DisplayDriver& display) {
    const char* status = nullptr;

    switch (heater.getState()) {
        case HeaterController::State::HEATING:   status = nullptr; break;  // Timer zeigt Status
        case HeaterController::State::COOLDOWN:  status = "COOLDOWN"; break;
        case HeaterController::State::ERROR:     status = "ERROR"; break;
        default:                     status = nullptr; break;
    }

    if (status) {
        //centerText(display, 180, status, TFT_WHITE, 2);
    }
}

void FireScreen::drawCycleInfo(DisplayDriver& display) {
    char text[20];
    snprintf(text, sizeof(text), "%d", state.currentCycle);
    display.drawBitmap(10, 76, image_fire_48, 48, 48, TFT_WHITE);
    display.drawText(60, 114, text, TFT_WHITE, 4);
}

void FireScreen::drawSessionStats(DisplayDriver& display) {
    char lineCaps[20];
    snprintf(lineCaps, sizeof(lineCaps), "%d", cachedCaps);

    char lineConsumption[20];
    snprintf(lineConsumption, sizeof(lineConsumption), "%.2fg", cachedConsumption);
    if (lineConsumption[0] == '0' && lineConsumption[1] == '.')
        memmove(lineConsumption, lineConsumption + 1, strlen(lineConsumption));

    char lineTodayConsumption[20];
    snprintf(lineTodayConsumption, sizeof(lineTodayConsumption), "%.2fg", cachedTodayConsumption);
    if (lineTodayConsumption[0] == '0' && lineTodayConsumption[1] == '.')
        memmove(lineTodayConsumption, lineTodayConsumption + 1, strlen(lineTodayConsumption));

    display.drawBitmap(160, 134,  (state.currentCycle == 1) ? image_cap_fill_48 : image_cap_48, 48, 48, TFT_WHITE);
    display.drawBitmap(10, 134, image_session_48, 48, 48, TFT_WHITE);


	display.drawText(213, 168, lineCaps, TFT_WHITE, 3);
	display.drawText(63, 168, lineConsumption, TFT_WHITE, 3);
	display.drawText(160, 108, lineTodayConsumption, TFT_WHITE, 3);
}

void FireScreen::update() {
    const bool isActive = heater.isHeating() || heater.getState() == HeaterController::State::COOLDOWN;

    // Update heating timer
    if (isActive) {
        state.lastActivityTime = millis();

        // Redraw timer jede Sekunde
        static uint32_t lastSecond = 0;
        const uint32_t currentSecond = (millis() - state.heatingStartTime) / 1000;
        if (currentSecond != lastSecond) {
            markDirty();
            lastSecond = currentSecond;
        }
    }

    checkScreensaverTimeout();

    // Handle touch input with debouncing
    int val = touchRead(32);
    state.currentTouchValue = val; // Store the raw touch value

    bool rawTouchActive;
    int effectiveTouchOnThreshold = state.touchOnThreshold;
    int effectiveTouchOffThreshold = state.touchOffThreshold;

    if (heater.isHeating()) {
        effectiveTouchOnThreshold += state.heaterInterferenceOffset;
        effectiveTouchOffThreshold += state.heaterInterferenceOffset;
    }

    // Hysteresis logic for rawTouchActive
    if (val < effectiveTouchOnThreshold) {
        rawTouchActive = true; // Definitely touched
    } else if (val > effectiveTouchOffThreshold) {
        rawTouchActive = false; // Definitely not touched
    } else {
        // In the ambiguous zone, maintain the previous raw state
        rawTouchActive = state.lastRawTouchActive;
    }

    if (rawTouchActive != state.lastRawTouchActive) {
        // Raw state has changed, reset the timer
        state.lastTouchChangeTime = millis();
    }

        if ((millis() - state.lastTouchChangeTime) > state.debounceDelay) {

            // Debounce time has passed, now check if the debounced state needs to change

            if (rawTouchActive != state.touchActive) {

                state.touchActive = rawTouchActive; // Update the debounced state

                markDirty(); // Mark dirty to redraw the 'X' or '_'

    

                if (DeviceState::instance().smart.get()) { // Only trigger heating if 'smart' mode is enabled

                    _handleHeatingTrigger(state.touchActive); // Start or stop heating based on touch state

                }

    

                if (state.touchActive) {

                    logPrint("Touch", "aktiviert");

                    Serial.println("Cap erkannt → ZVS EIN");

                } else {

                    Serial.println("val: " + String(val)); // Log value when touch deactivates

                }

            }

        }

        state.lastRawTouchActive = rawTouchActive; // Store the raw state for the next iteration
}

void FireScreen::handleInput(InputEvent event) {
    if (event.type != PRESS) return;

    resetActivityTimer();

    // Determine if heating should be triggered
    bool triggerHeating = false;
    if (event.button == FIRE) {
        triggerHeating = true; // FIRE button always triggers heating
    } else if (event.button == CENTER && DeviceState::instance().enableCenterButtonForHeating.get()) {
        triggerHeating = true; // CENTER button triggers heating if enabled
    }

    if (triggerHeating) {
        _handleHeatingTrigger(!heater.isHeating()); // Toggle heating state
        return; // Consume the event if it triggered heating
    }

    // Handle other inputs (UP/DOWN for cycle change)
    switch (event.button) {
        case UP:
            handleCycleChange();
            break;

        case DOWN:
            handleCycleChange();
            break;

        default:
            break;
    }
}

void FireScreen::handleCycleChange() {
    if (state.currentCycle == 1) {
        state.currentCycle = 2;
    } else {
        state.currentCycle = 1;
    }

    setCycleCallback(state.currentCycle);
    markDirty();
}

void FireScreen::checkScreensaverTimeout() {
    const bool isActive = heater.isHeating() || heater.getState() == HeaterController::State::COOLDOWN;

    if (!isActive && (millis() - state.lastActivityTime > Timing::SCREENSAVER_TIMEOUT_MS)) {
        screenManager->setScreen(screensaverScreen, ScreenTransition::FADE);
    }
}

void FireScreen::_handleHeatingTrigger(bool shouldStartHeating) {
    if (shouldStartHeating) {
        if (!heater.isHeating()) {
            heater.startHeating();
            state.heatingStartTime = millis();
        }
    } else {
        if (heater.isHeating()) {
            const bool updateCycle = heater.isHeating(); // This will always be true here
            heater.stopHeating();
            if (updateCycle && (millis() - state.heatingStartTime > 10000)) {
                setCycleCallback(state.currentCycle);
                state.currentCycle = (state.currentCycle == 1) ? 2 : 1;
            }
        }
    }
    markDirty(); // Always mark dirty when heating state changes
}

void FireScreen::resetActivityTimer() {
    state.lastActivityTime = millis();
}
