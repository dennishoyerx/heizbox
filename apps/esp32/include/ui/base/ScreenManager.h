#pragma once

#include "ui/base/Screen.h"
#include "ui/base/UI.h"
#include "ui/components/StatusBar.h"
#include "hardware/input/InputManager.h"
#include "hardware/display/DisplayDriver.h"
#include "ui/base/ScreenTransition.h"

class ScreenManager {
public:
    ScreenManager(DisplayDriver& disp, InputManager& inp);
    ~ScreenManager() = default;

    // Lifecycle
    void update();
    void draw();
    void handleInput(InputEvent event);

    // Screen management
    void setScreen(Screen* newScreen, ScreenTransition transition = ScreenTransition::NONE);
    Screen* getCurrentScreen() const { return currentScreen; }
    ScreenType getCurrentScreenType() const { return currentScreenType; }
    void registerScreen(ScreenType type, Screen* screen);
    Screen* getScreen(ScreenType type);
    void switchScreen(ScreenType screenType, ScreenTransition transition = ScreenTransition::NONE);

    // Dirty flag für Re-Rendering
    void setDirty() { dirty = true; }
    bool isDirty() const { return dirty; }

private:
    // Core components
    DisplayDriver& display;
    InputManager& input;
    StatusBar* statusBar;
    UI* ui;

    // Screen state
    Screen* currentScreen;
    Screen* previousScreen;  // Für Transitions
    ScreenType currentScreenType;
    std::unordered_map<ScreenType, Screen*> screens_;

    // Rendering state
    bool dirty;
    uint32_t lastDrawTime;

    // Transition state
    struct TransitionState {
        ScreenTransition type;
        bool inProgress;
        uint32_t startTime;
        uint8_t progress;  // 0-100
        uint8_t originalBrightness;

        static constexpr uint32_t DURATION_MS = 200;
    } transition;

    // Helper methods
    void performTransition();
    void completeTransition();
    void drawTransitionFrame();
};
