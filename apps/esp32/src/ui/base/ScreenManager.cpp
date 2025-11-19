// src/ScreenManager.cpp
#include "utils/Logger.h"
#include "ui/base/ScreenManager.h"
#include "hardware/display/DisplayDriver.h"
#include "ui/base/UI.h"

#include "hardware/input/InputManager.h"
#include "ui/base/ScreenTransition.h"
ScreenManager::ScreenManager(DisplayDriver& disp, InputManager& inp)
    : display(disp),
      input(inp),
      currentScreen(nullptr),
      previousScreen(nullptr),
      currentScreenType(ScreenType::STARTUP),
      dirty(true),
      lastDrawTime(0)
{
    ui = new UI(&disp);
    transition.type = ScreenTransition::NONE;
    transition.inProgress = false;
    transition.startTime = 0;
    transition.progress = 0;
}

// ============================================================================
// Screen Management
// ============================================================================

void ScreenManager::setScreen(Screen* newScreen, ScreenTransition transitionType) {
    if (newScreen == currentScreen) return;

    // Store previous screen für Transition
    previousScreen = currentScreen;
    currentScreen = newScreen;
    dirty = true;

    if (currentScreen) {
        currentScreenType = currentScreen->getType();
        currentScreen->setManager(this);
        currentScreen->setUI(ui);

        // Initialize transition wenn gewünscht
        if (transitionType != ScreenTransition::NONE && previousScreen) {
            transition.type = transitionType;
            transition.inProgress = true;
            transition.startTime = millis();
            transition.progress = 0;
            transition.originalBrightness = display.getBrightness();
        } else {
            // Direkter Wechsel 
            currentScreen->clear();
            currentScreen->draw(display);
            display.renderStatusBar();
            transition.inProgress = false;
        }
    }

    Serial.printf("\u2195 Screen changed to: %d\n", static_cast<int>(currentScreenType));
}
void ScreenManager::registerScreen(ScreenType type, Screen* screen) {
    screens_[type] = screen;
}
Screen* ScreenManager::getScreen(ScreenType type) {
    auto it = screens_.find(type);
    if (it != screens_.end()) {
        return it->second;
    }
    return nullptr;
}

// ============================================================================
// Update & Draw Loop
// ============================================================================

void ScreenManager::update() {
    if (!currentScreen) return;

    // Update transition wenn aktiv
    if (transition.inProgress) {
        performTransition();
        return;  // Kein Screen-Update während Transition
    }

    // Normales Screen-Update
    currentScreen->update();
}

void ScreenManager::draw() {
    if (!currentScreen) return;

    // Draw transition frame
    if (transition.inProgress) {
        drawTransitionFrame();
        return;
    }

    // Normales Rendering (nur wenn dirty)
    if (dirty) {
        const uint32_t startTime = micros();

        currentScreen->draw(display);
        display.render();
        display.renderStatusBar();

        const uint32_t drawTime = micros() - startTime;
        lastDrawTime = drawTime;
        dirty = false;

        // Performance-Warnung bei langsamen Draws
        if (drawTime > 50000) {  // > 50ms
            Serial.printf("\u26a0 Slow draw: %lu \u00b5s\n", drawTime);
        }
    }
}

void ScreenManager::handleInput(InputEvent event) {
    if (transition.inProgress) return;  // Ignore input während Transition

    if (currentScreen) {
        currentScreen->handleInput(event);
    }
}

// ============================================================================
// Transition Logic
// ============================================================================

void ScreenManager::performTransition() {
    const uint32_t elapsed = millis() - transition.startTime;

    // Calculate progress (0-100)
    transition.progress = min(100U, (elapsed * 100) / TransitionState::DURATION_MS);

    // Mark dirty für next draw
    dirty = true;

    // Check completion
    if (transition.progress >= 100) {
        completeTransition();
    }
}

void ScreenManager::drawTransitionFrame() {
    switch (transition.type) {
        case ScreenTransition::FADE:
            // Simplified fade: Brightness-basiert
            {
                const uint8_t brightness = transition.originalBrightness;
                const uint8_t targetBrightness = (transition.progress < 50)
                    ? map(transition.progress, 0, 50, brightness, 0)
                    : map(transition.progress, 50, 100, 0, brightness);

                display.setBrightness(targetBrightness);

                // Bei 50% Screen wechseln
                if (transition.progress >= 50 && previousScreen) {
                    //display.clear();
                    currentScreen->draw(display);
                    display.render();
                    previousScreen = nullptr;
                }
            }
            break;

        case ScreenTransition::NONE:
        default:
            // Direkt
            completeTransition();
            break;
    }
}

void ScreenManager::completeTransition() {
    transition.inProgress = false;
    previousScreen = nullptr;

    // Restore brightness
    display.setBrightness(transition.originalBrightness);

    if (currentScreen) {
        currentScreen->clear();
        currentScreen->draw(display);
    }
    display.renderStatusBar();

    dirty = false;
}
