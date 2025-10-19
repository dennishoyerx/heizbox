// include/ScreenManager.h
#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "Screen.h"
#include "InputManager.h"
#include "DisplayManager.h"
#include "ScreenType.h"

// Transition-Effekte für Screen-Wechsel
enum class ScreenTransition : uint8_t {
    NONE,           // Direkt wechseln
    FADE,           // Fade out/in (falls genug RAM)
    SLIDE_LEFT,     // Slide-Animation (zukünftig)
    SLIDE_RIGHT
};

class ScreenManager {
public:
    ScreenManager(DisplayManager& disp, InputManager& inp);
    ~ScreenManager() = default;

    // Lifecycle
    void update();
    void draw();
    void handleInput(InputEvent event);

    // Screen management
    void setScreen(Screen* newScreen, ScreenTransition transition = ScreenTransition::NONE);
    Screen* getCurrentScreen() const { return currentScreen; }
    ScreenType getCurrentScreenType() const { return currentScreenType; }

    // Dirty flag für Re-Rendering
    void setDirty() { dirty = true; }
    bool isDirty() const { return dirty; }

private:
    // Core components
    DisplayManager& display;
    InputManager& input;

    // Screen state
    Screen* currentScreen;
    Screen* previousScreen;  // Für Transitions
    ScreenType currentScreenType;

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

#endif
