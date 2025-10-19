// include/Screen.h
#ifndef SCREEN_H
#define SCREEN_H

#include <unordered_map>
#include <string>
#include "ScreenType.h"

class DisplayManager;
class ScreenManager;
struct InputEvent;

// Base Screen mit Common Functionality
class Screen {
public:
    Screen();
    virtual ~Screen();

    // Pure virtual (muss implementiert werden)
    virtual void draw(DisplayManager& display) = 0;
    virtual void update() = 0;
    virtual void handleInput(InputEvent event) = 0;
    virtual ScreenType getType() const = 0;

    // Optional overrides
    virtual void onEnter() {}  // Called when screen becomes active
    virtual void onExit() {}   // Called when screen is left
    virtual bool needsRedraw() const { return false; }

    // State management (einfacher Key-Value Store)
    void setState(const std::string& key, int value);
    int getState(const std::string& key, int defaultValue = 0) const;

    // Manager access
    void setManager(ScreenManager* mgr);

protected:
    virtual void initState() {}
    ScreenManager* manager;

    // Helper für häufige Operationen
    void markDirty();
    void centerText(DisplayManager& display, int16_t y, const char* text,
                    uint16_t color, uint8_t size = 2);

private:
    std::unordered_map<std::string, int> state_;
};

#endif
