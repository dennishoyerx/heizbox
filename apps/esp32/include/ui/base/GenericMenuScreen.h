// include/ui/base/GenericMenuScreen.h
#pragma once

#include <vector>
#include <memory>
#include "ui/base/Screen.h"
#include "ui/components/MenuBuilder.h"
#include "hardware/DisplayDriver.h"
#include "hardware/InputManager.h"

class GenericMenuScreen : public Screen {
public:
    GenericMenuScreen(const char* title, std::vector<std::unique_ptr<MenuItem>> items)
        : title_(title), items_(std::move(items)), selectedIndex_(0),
          adjustMode_(false) {}

    void draw(DisplayDriver& display) override;

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
