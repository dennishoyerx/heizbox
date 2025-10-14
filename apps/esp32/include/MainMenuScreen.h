#ifndef MAINMENUSCREEN_H
#define MAINMENUSCREEN_H

#include <vector>
#include <functional>
#include "Screen.h"
#include "DisplayManager.h"
#include "ScreenManager.h"
#include "ScreenType.h"
#include "InputManager.h"

class DisplayManager;

struct InputEvent;

struct MenuItem {
    const char* title;
    std::function<void()> action;
};

class TimezoneScreen; // Forward declaration

class MainMenuScreen : public Screen {
private:
    std::vector<MenuItem> items;
    int selectedIndex;
    DisplayManager* display;
    ScreenManager* screenManager;
    Screen* statsScreen = nullptr;
    TimezoneScreen* timezoneScreen = nullptr;

public:
    MainMenuScreen(DisplayManager* display, ScreenManager* screenManager);
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;

    void adjustBrightness();
    void configureAutoStop();
    void configureSleepTimeout();
    void enterHiddenMode();

    void setStatsScreen(Screen* screen);
    void setTimezoneScreen(TimezoneScreen* screen);
};

#endif