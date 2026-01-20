// include/MenuBuilder.h
#ifndef MENUBUILDER_H
#define MENUBUILDER_H

#include <vector>
#include <functional>
#include <memory>
#include <utility> // For std::make_unique
#include <Arduino.h> // For constrain
#include "ui/base/Screen.h" // CONVERTED: Use forward.h
#include "ui/base/ScreenManager.h" // For ScreenTransition
#include "DisplayDriver.h" // For DisplayManager and TFT_ colors
#include "driver/input/InputManager.h" // For InputEvent
  // For Observable

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
    HEADLINE,
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

// ============================================================================
// Observable Range Menu Item - Direkte Observable-Integration
// ============================================================================

template<typename T>
class ObservableRangeMenuItem : public MenuItem {
public:
    ObservableRangeMenuItem(const char* title, Observable<T>& observable,
                           T minVal, T maxVal, T step,
                           const char* unit = nullptr)
        : title_(title), observable_(observable),
          minVal_(minVal), maxVal_(maxVal), step_(step), unit_(unit) {
        valueBuffer_[0] = '\0';
    }
    
    const char* getTitle() const override { return title_; }
    
    const char* getValue() const override {
        snprintf(const_cast<char*>(valueBuffer_), sizeof(valueBuffer_),
                 "%d%s", static_cast<int>(observable_.get()), 
                 unit_ ? unit_ : "");
        return valueBuffer_;
    }
    
    MenuItemType getType() const override { return MenuItemType::RANGE; }
    
    void execute() override {}
    
    void adjust(int delta) override {
        T current = observable_.get();
        T newVal = current + (delta * step_);
        newVal = constrain(newVal, minVal_, maxVal_);
        observable_.set(newVal);
    }

private:
    const char* title_;
    Observable<T>& observable_;
    T minVal_, maxVal_, step_;
    const char* unit_;
    mutable char valueBuffer_[16];
};

// Observable Toggle Menu Item
class ObservableToggleMenuItem : public MenuItem {
public:
    ObservableToggleMenuItem(const char* title, Observable<bool>& observable)
        : title_(title), observable_(observable) {}
    
    const char* getTitle() const override { return title_; }
    const char* getValue() const override { 
        return observable_.get() ? "ON" : "OFF"; 
    }
    MenuItemType getType() const override { return MenuItemType::TOGGLE; }
    
    void execute() override {
        observable_.set(!observable_.get());
    }

private:
    const char* title_;
    Observable<bool>& observable_;
};

class NavigationMenuItem : public MenuItem {
public:
    NavigationMenuItem(const char* title, Screen* targetScreen, ScreenManager* manager)
        : title_(title), targetScreen_(targetScreen), manager_(manager) {}
    
    const char* getTitle() const override { return title_; }
    MenuItemType getType() const override { return MenuItemType::SCREEN; }
    
    void execute() override {
        if (manager_ && targetScreen_) {
            manager_->setScreen(targetScreen_, ScreenTransition::NONE);
        }
    }

private:
    const char* title_;
    Screen* targetScreen_;
    ScreenManager* manager_;
};

class HeadlineMenuItem : public MenuItem {
public:
    HeadlineMenuItem(const char* title)
        : title_(title) {}
    
    const char* getTitle() const override { return title_; }
    MenuItemType getType() const override { return MenuItemType::HEADLINE; }
    
    void execute() override {    }

private:
    const char* title_;
};

// ============================================================================
// Observable Range mit Display-Converter (z.B. ms → s)
// ============================================================================

template<typename T>
class ObservableRangeMenuItemWithConverter : public MenuItem {
public:
    using DisplayConverter = std::function<int(T)>;
    using StoreConverter = std::function<T(int)>;
    
    ObservableRangeMenuItemWithConverter(const char* title, Observable<T>& observable,
                                        T minVal, T maxVal, T step,
                                        const char* unit,
                                        DisplayConverter displayConv,
                                        StoreConverter storeConv)
        : title_(title), observable_(observable),
          minVal_(minVal), maxVal_(maxVal), step_(step), unit_(unit),
          displayConv_(displayConv), storeConv_(storeConv) {}
    
    const char* getTitle() const override { return title_; }
    
    const char* getValue() const override {
        int displayValue = displayConv_(observable_.get());
        snprintf(const_cast<char*>(valueBuffer_), sizeof(valueBuffer_),
                 "%d%s", displayValue, unit_ ? unit_ : "");
        return valueBuffer_;
    }
    
    MenuItemType getType() const override { return MenuItemType::RANGE; }
    void execute() override {}
    
    void adjust(int delta) override {
        T current = observable_.get();
        T newVal = current + (delta * step_);
        newVal = constrain(newVal, minVal_, maxVal_);
        observable_.set(newVal);
    }

private:
    const char* title_;
    Observable<T>& observable_;
    T minVal_, maxVal_, step_;
    const char* unit_;
    DisplayConverter displayConv_;
    StoreConverter storeConv_;
    mutable char valueBuffer_[16];
};

// ============================================================================
// Menu Builder
// ============================================================================

class MenuBuilder {
public:
    MenuBuilder() = default;

    MenuBuilder& addHeadline(const char* title) {
        items_.emplace_back(std::make_unique<HeadlineMenuItem>(title));
        return *this;
    }
    
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

    MenuBuilder& addRange(const char* title,
                        std::function<void(int)> onChange,
                        int minVal, int maxVal, int step = 1,
                        const char* unit = nullptr) {
        items_.emplace_back(std::make_unique<RangeMenuItem>(
            title, nullptr, minVal, maxVal, step, unit, std::move(onChange)));
        return *this;
    }

    // Observable-Varianten
    template<typename T>
    MenuBuilder& addObservableRange(const char* title, Observable<T>& observable,
                                    T minVal, T maxVal, T step = 1,
                                    const char* unit = nullptr) {
        items_.emplace_back(std::make_unique<ObservableRangeMenuItem<T>>(
            title, observable, minVal, maxVal, step, unit));
        return *this;
    }
    
    MenuBuilder& addObservableToggle(const char* title, Observable<bool>& observable) {
        items_.emplace_back(std::make_unique<ObservableToggleMenuItem>(
            title, observable));
        return *this;
    }
    
    // Helper für PersistedObservable mit automatischer Unit-Konvertierung
    MenuBuilder& addObservableRangeMs(
        const char* title,
        Observable<uint32_t>& observable,
        uint32_t minMs, uint32_t maxMs, uint32_t stepMs,
        bool showInMs = false
    ) {
        if (showInMs) {
            // Anzeige in ms
            items_.emplace_back(std::make_unique<
                ObservableRangeMenuItemWithConverter<uint32_t>
            >(title, observable, minMs, maxMs, stepMs,
            "ms",
            [](uint32_t v){ return v; },
            [](uint32_t v){ return v; }));
        } else {
            // Anzeige in Sekunden
            items_.emplace_back(std::make_unique<
                ObservableRangeMenuItemWithConverter<uint32_t>
            >(title, observable, minMs, maxMs, stepMs,
            "s",
            [](uint32_t ms){ return ms / 1000; },
            [](uint32_t s ){ return s * 1000; }));
        }

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

#include "ui/base/GenericMenuScreen.h"

#endif // MENUBUILDER_H