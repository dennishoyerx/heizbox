#ifndef SCREEN_H
#define SCREEN_H

#include <unordered_map>
#include <string>
#include "ScreenType.h"

// Forward declarations
class DisplayManager;
class ScreenManager;
struct InputEvent;

class Screen {
public:
    Screen();
    virtual ~Screen();
    virtual void draw(DisplayManager& display) = 0;
    virtual void update() = 0;
    virtual void handleInput(InputEvent event) = 0;
    virtual ScreenType getType() const = 0;
    virtual bool needsRedraw() const { return false; }

    void setState(const std::string& key, int value);
    int getState(const std::string& key, int defaultValue = 0) const;
    void setManager(ScreenManager* mgr);

protected:
    virtual void initState() {}
    ScreenManager* manager;

private:
    std::unordered_map<std::string, int> state_;
};

#endif