#pragma once
#include <string>
#include <vector>
#include <memory>
#include <Arduino.h>

class IMenuItem {
public:
    virtual ~IMenuItem() = default;
    virtual String name() const = 0;
    virtual String valueString() const = 0;
    virtual void increment() = 0;
    virtual void decrement() = 0;
    virtual void onEnter() {}
};

using MenuItemPtr = std::unique_ptr<IMenuItem>;

class MenuManager {
public:
    void addItem(MenuItemPtr item) { items_.push_back(std::move(item)); }
    void nextOption();
    void prevOption();
    void increment();
    void decrement();

    const IMenuItem* current() const {
        if (items_.empty()) return nullptr;
        return items_[currentIndex_].get();
    }
    const IMenuItem* at(size_t idx) const {
        if (idx >= items_.size()) return nullptr;
        return items_[idx].get();
    }
    size_t index() const { return currentIndex_; }
    size_t count() const { return items_.size(); }

private:
    std::vector<MenuItemPtr> items_;
    size_t currentIndex_ = 0;
};