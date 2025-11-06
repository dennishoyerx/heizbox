#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "hardware/DisplayDriver.h"

class DisplayDriver;

// ---------------- Base ----------------

class UIBaseComponent {
public:
    virtual ~UIBaseComponent() = default; // notwendig für dynamic_cast
    int x = 0;
    int y = 0;
    uint16_t color = 0xFFFF;
    bool visible = true;
    bool dirty = false;

    void setPosition(int x, int y);
};

// ---------------- Text ----------------

class UICText : public UIBaseComponent {
public:
    std::string text;
    int size = 1;

    void setText(const std::string& text);
    void draw(DisplayDriver& display);
};

// ---------------- Icon ----------------

class UICIcon : public UIBaseComponent {
public:
    std::string image;
    int width = 0;
    int height = 0;
};

// ---------------- Manager ----------------

class UIManager {
public:
    UICText Text(const std::string& text, int x, int y);
    UICText* Text(const std::string& id, const std::string& text);

private:
    std::unordered_map<std::string, std::unique_ptr<UIBaseComponent>> components;

    void registerUIComponent(const std::string& id, std::unique_ptr<UIBaseComponent> component);
    void unregisterUIComponent(const std::string& id);
    UIBaseComponent* getComponent(const std::string& id);
};

#endif
