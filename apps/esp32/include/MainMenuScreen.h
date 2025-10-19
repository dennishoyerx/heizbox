// include/MainMenuScreen.h
#ifndef MAINMENUSCREEN_H
#define MAINMENUSCREEN_H

#include "Screen.h"
#include "DisplayManager.h"
#include "ScreenManager.h"
#include "TimezoneScreen.h"
#include <vector>
#include <string>
#include <functional>

struct MenuItem {
    std::string title;
    std::function<void()> action;
};

class MainMenuScreen : public Screen {
public:
    MainMenuScreen(DisplayManager* display, ScreenManager* screenManager);

    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;

    void setStatsScreen(Screen* screen);
    void setTimezoneScreen(TimezoneScreen* screen);

private:
    size_t selectedIndex;
    std::vector<MenuItem> items;
    DisplayManager* display;
    ScreenManager* screenManager;
    Screen* statsScreen;
    TimezoneScreen* timezoneScreen;

    void adjustBrightness();
    void configureAutoStop();
    void configureSleepTimeout();
    void enterHiddenMode();
};

#endif
