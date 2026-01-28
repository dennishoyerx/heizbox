#include "utils/Logger.h"
#include "ui/base/ScreenManager.h"
#include "DisplayDriver.h"
#include "ui/base/UI.h"
#include "Config.h"

#include "driver/input/InputManager.h"

ScreenManager::ScreenManager(DisplayDriver& disp, InputManager& inp)
    : display(disp),
      input(inp),
      currentScreen(nullptr),
      currentScreenType(ScreenType::STARTUP),
      dirty(true),
      ui(new UI(&disp)),
      statusBar(new StatusBar(DisplayConfig::WIDTH, DisplayConfig::STATUS_BAR_HEIGHT)) {}

// ============================================================================
// Screen Management
// ============================================================================

void ScreenManager::setScreen(Screen* newScreen) {
    if (newScreen == currentScreen) return;

    currentScreen = newScreen;
    dirty = true;

    if (!currentScreen) return;
    currentScreenType = currentScreen->getType();
    currentScreen->setManager(this);
    currentScreen->setUI(ui);
    ui->invalidateAll();
    ui->clear();
        
    currentScreen->clear();
    currentScreen->draw();
    //statusBar->draw(ui);

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

void ScreenManager::switchScreen(ScreenType screenType) {
    Screen* screenToSwitch = getScreen(screenType);
    if (screenToSwitch) setScreen(screenToSwitch);
    else logPrint("ScreenManager", "Attempted to switch to unregistered screen type: %d", static_cast<int>(screenType));
}

// ============================================================================
// Update & Draw Loop
// ============================================================================

void ScreenManager::update() {
    if (currentScreen) currentScreen->update();
}

void ScreenManager::draw() {
    if (!currentScreen || !dirty) return;
    const uint32_t startTime = micros();

    currentScreen->draw();
    if (statusbarVisible) statusBar->draw(ui);
    const uint32_t drawTime = micros() - startTime;
    dirty = false;

    // Performance-Warnung bei langsamen Draws
    if (drawTime > 50000)Serial.printf("\u26a0 Slow draw: %lu \u00b5s\n", drawTime);
}

void ScreenManager::handleInput(InputEvent event) {
    if (!currentScreen) return;
    currentScreen->handleInput(event);
    dirty = true;
}

