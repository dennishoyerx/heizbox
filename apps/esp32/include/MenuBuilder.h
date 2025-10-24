// include/MenuBuilder.h
#ifndef MENUBUILDER_H
#define MENUBUILDER_H

#include <vector>
#include <functional>
#include <memory>
#include <utility> // For std::make_unique
#include <Arduino.h> // For constrain
#include "Screen.h"
#include "ScreenManager.h" // For ScreenTransition
#include "DisplayManager.h" // For DisplayManager and TFT_ colors
#include "InputManager.h" // For InputEvent

// Custom make_unique for C++11 compatibility if needed
#if __cplusplus < 201402L
namespace std {
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif

// ============================================================================
// Menu Item Types
// ============================================================================

enum class MenuItemType : uint8_t {
    ACTION,      // Führt direkt eine Aktion aus
    TOGGLE,      // Boolean Toggle
    RANGE,       // Wert mit Min/Max
    SUBMENU,     // Zu einem anderen Menü navigieren
    SCREEN       // Zu einem Screen navigieren
};

// ============================================================================
// Menu Item Base
// ============================================================================

class MenuItem {
public:
    virtual ~MenuItem() = default;
    
    virtual const char* getTitle() const = 0;
    virtual const char* getValue() const { return nullptr; }
    virtual MenuItemType getType() const = 0;
    virtual void execute() = 0;
    virtual void adjust(int delta) {}
    virtual bool isEnabled() const { return true; }
};

// ============================================================================
// Concrete Menu Items
// ============================================================================

class ActionMenuItem : public MenuItem {
public:
    ActionMenuItem(const char* title, std::function<void()> action)
        : title_(title), action_(std::move(action)) {}
    
    const char* getTitle() const override { return title_; }
    MenuItemType getType() const override { return MenuItemType::ACTION; }
    void execute() override { if (action_) action_(); }

private:
    const char* title_;
    std::function<void()> action_;
};

class ToggleMenuItem : public MenuItem {
public:
    ToggleMenuItem(const char* title, bool* valuePtr, 
                   std::function<void(bool)> onChange = nullptr)
        : title_(title), valuePtr_(valuePtr), onChange_(std::move(onChange)) {}
    
    const char* getTitle() const override { return title_; }
    const char* getValue() const override { return *valuePtr_ ? "ON" : "OFF"; }
    MenuItemType getType() const override { return MenuItemType::TOGGLE; }
    
    void execute() override {
        *valuePtr_ = !*valuePtr_;
        if (onChange_) onChange_(*valuePtr_);
    }

private:
    const char* title_;
    bool* valuePtr_;
    std::function<void(bool)> onChange_;
};

class RangeMenuItem : public MenuItem {
public:
    RangeMenuItem(const char* title, int* valuePtr, 
                  int minVal, int maxVal, int step,
                  const char* unit = nullptr,
                  std::function<void(int)> onChange = nullptr)
        : title_(title), valuePtr_(valuePtr), 
          minVal_(minVal), maxVal_(maxVal), step_(step),
          unit_(unit), onChange_(std::move(onChange)) {
        valueBuffer_[0] = '\0';
    }
    
    const char* getTitle() const override { return title_; }
    
    const char* getValue() const override {
        snprintf(const_cast<char*>(valueBuffer_), sizeof(valueBuffer_), 
                 "%d%s", *valuePtr_, unit_ ? unit_ : "");
        return valueBuffer_;
    }
    
    MenuItemType getType() const override { return MenuItemType::RANGE; }
    
    void execute() override {} // Range items are adjusted, not executed
    
    void adjust(int delta) override {
        int newVal = *valuePtr_ + (delta * step_);
        *valuePtr_ = constrain(newVal, minVal_, maxVal_);
        if (onChange_) onChange_(*valuePtr_);
    }

private:
    const char* title_;
    int* valuePtr_;
    int minVal_, maxVal_, step_;
    const char* unit_;
    std::function<void(int)> onChange_;
    mutable char valueBuffer_[16];
};

class NavigationMenuItem : public MenuItem {
public:
    NavigationMenuItem(const char* title, Screen* targetScreen, ScreenManager* manager)
        : title_(title), targetScreen_(targetScreen), manager_(manager) {}
    
    const char* getTitle() const override { return title_; }
    MenuItemType getType() const override { return MenuItemType::SCREEN; }
    
    void execute() override {
        if (manager_ && targetScreen_) {
            manager_->setScreen(targetScreen_, ScreenTransition::FADE);
        }
    }

private:
    const char* title_;
    Screen* targetScreen_;
    ScreenManager* manager_;
};

// ============================================================================
// Menu Builder
// ============================================================================

class MenuBuilder {
public:
    MenuBuilder() = default;
    
    MenuBuilder& addAction(const char* title, std::function<void()> action) {
        items_.emplace_back(std::make_unique<ActionMenuItem>(title, std::move(action)));
        return *this;
    }
    
    MenuBuilder& addToggle(const char* title, bool* valuePtr, 
                          std::function<void(bool)> onChange = nullptr) {
        items_.emplace_back(std::make_unique<ToggleMenuItem>(title, valuePtr, std::move(onChange)));
        return *this;
    }
    
    MenuBuilder& addRange(const char* title, int* valuePtr,
                         int minVal, int maxVal, int step = 1,
                         const char* unit = nullptr,
                         std::function<void(int)> onChange = nullptr) {
        items_.emplace_back(std::make_unique<RangeMenuItem>(
            title, valuePtr, minVal, maxVal, step, unit, std::move(onChange)));
        return *this;
    }
    
    MenuBuilder& addNavigation(const char* title, Screen* screen, ScreenManager* manager) {
        items_.emplace_back(std::make_unique<NavigationMenuItem>(title, screen, manager));
        return *this;
    }
    
    std::vector<std::unique_ptr<MenuItem>> build() {
        return std::move(items_);
    }

private:
    std::vector<std::unique_ptr<MenuItem>> items_;
};

// ============================================================================
// Generic Menu Screen
// ============================================================================

class GenericMenuScreen : public Screen {
public:
    GenericMenuScreen(const char* title, std::vector<std::unique_ptr<MenuItem>> items)
        : title_(title), items_(std::move(items)), selectedIndex_(0), 
          adjustMode_(false) {}
    
    void draw(DisplayManager& display) override {
        display.clear();
        
        // Title
        centerText(display, 10, title_, TFT_WHITE, 2);
        
        // Items (with scrolling support)
        const int itemsPerPage = 5;
        const int startIdx = (selectedIndex_ / itemsPerPage) * itemsPerPage;
        const int endIdx = std::min(startIdx + itemsPerPage, static_cast<int>(items_.size()));
        
        for (int i = startIdx; i < endIdx; i++) {
            const int displayIdx = i - startIdx;
            const int16_t y = 50 + displayIdx * 30;
            
            const auto& item = items_[i];
            const bool isSelected = (i == selectedIndex_);
            const uint16_t color = isSelected ? TFT_YELLOW : TFT_WHITE;
            
            // Selection indicator
            if (isSelected) {
                display.drawText(10, y, adjustMode_ ? ">" : "*", color, 2);
            }
            
            // Item title
            display.drawText(30, y, item->getTitle(), color, 2);
            
            // Item value (if any)
            const char* value = item->getValue();
            if (value) {
                const int16_t valueX = 200;
                display.drawText(valueX, y, value, color, 2);
            }
        }
        
        // Footer
        const char* footer = adjustMode_ 
            ? "L/R: Adjust  OK: Done"
            : "OK: Select  HOLD L: Back";
        display.drawText(10, 210, footer, TFT_GRAY, 1);
    }
    
    void update() override {}
    
    void handleInput(InputEvent event) override;
    
    ScreenType getType() const override { return ScreenType::MAIN_MENU; }

private:
    void handleNavigationMode(InputEvent event);
    void handleAdjustMode(InputEvent event);
    
    const char* title_;
    std::vector<std::unique_ptr<MenuItem>> items_;
    size_t selectedIndex_;
    bool adjustMode_;
};

#endif // MENUBUILDER_H
