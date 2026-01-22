#pragma once


#include "ui/base/Screen.h"
#include "ui/base/UI.h"
#include "ui/components/StatusBar.h"
#include "driver/input/InputManager.h"
#include "DisplayDriver.h"

class ScreenManager {
public:
    ScreenManager(DisplayDriver& disp, InputManager& inp);
    ~ScreenManager() = default;

    // Lifecycle
    void update();
    void draw();
    void handleInput(InputEvent event);

    // Screen management
    void setScreen(Screen* newScreen);
    Screen* getCurrentScreen() const { return currentScreen; }
    ScreenType getCurrentScreenType() const { return currentScreenType; }
    void registerScreen(ScreenType type, Screen* screen);
    Screen* getScreen(ScreenType type);
    void switchScreen(ScreenType screenType);

    // Dirty flag für Re-Rendering
    void setDirty() { dirty = true; }
    bool isDirty() const { return dirty; }

    UI* getUI() const { return ui; }

    void setStatusbarVisible(bool visible) { statusbarVisible = visible; }
    

private:
    // Core components
    DisplayDriver& display;
    InputManager& input;
    StatusBar* statusBar;
    UI* ui;

    // Screen state
    Screen* currentScreen;
    ScreenType currentScreenType;
    std::unordered_map<ScreenType, Screen*> screens_;

    // Rendering state
    bool dirty;
    bool statusbarVisible = false;
};
