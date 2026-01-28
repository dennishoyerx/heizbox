#include "ui/base/GenericMenuScreen.h"
#include "driver/input/InputManager.h"
#include "ui/base/ScreenManager.h" 
#include "ui/ColorPalette.h"
#include <algorithm> // For std::min


GenericMenuScreen::GenericMenuScreen(const char* title, std::vector<std::unique_ptr<MenuItem>> items) : 
        title_(title), items_(std::move(items)), selectedIndex_(0), adjustMode_(false) {
    selectedIndex_ = 0;
    /*while (selectedIndex_ < items_.size() && items_[selectedIndex_]->getType() == MenuItemType::HEADLINE) {
        selectedIndex_++;
    }*/

    // Fallback, falls alle HEADLINEs
    if (selectedIndex_ >= items_.size()) selectedIndex_ = 0;
}

void GenericMenuScreen::draw() {
    if (manager->isStatusbarVisible()) manager->setStatusbarVisible(false);
    _ui->clear();    
    _ui->withSurface(280, 240, 0, 0, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
    
    // Items (with scrolling support)
    const int itemsPerPage = 8;
    const int startIdx = (selectedIndex_ / itemsPerPage) * itemsPerPage;
    const int endIdx = std::min(startIdx + itemsPerPage, static_cast<int>(items_.size()));
    
    for (int i = startIdx; i < endIdx; i++) {
        const int displayIdx = i - startIdx;
        const int16_t y = 30 + displayIdx * 30;
        
        const auto& item = items_[i];
        const bool isSelected = (i == selectedIndex_);
        const uint8_t color = isSelected ? adjustMode_ ? COLOR_ACCENT : COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY;
        
        // Selection indicator

        // Item title
        s.text(10, y, item->getTitle(), TextSize::sm, color);

        s.text(0, 0, "", {.align = TextAlign::center});
        
        // Item value (if any)
        const char* value = item->getValue();
        if (value) s.text(200, y, value, TextSize::md, color);
    }
    return;
    // Footer
    const char* footer = adjustMode_ 
        ? "L/R: Adjust  OK: Done"
        : "OK: Select  HOLD L: Back";
        s.text(30, 210, footer, TextSize::sm);
    });
}

void GenericMenuScreen::handleInput(InputEvent event) {
    if (adjustMode_ && input(event, {UP, DOWN, ROTARY_ENCODER, CENTER}, {PRESSED, HOLD, ROTARY_CW, ROTARY_CCW})) {
        handleAdjustMode(event);
    } else if (
        input(event, {UP, DOWN}, {PRESSED, HOLD}) || 
        input(event, {CENTER}, {PRESSED}) || 
        input(event, {ROTARY_ENCODER}, {ROTARY_CW, ROTARY_CCW})) {
        handleNavigationMode(event);
    }
}

void GenericMenuScreen::handleNavigationMode(InputEvent event) {
    if (event.button == DOWN || event.type == ROTARY_CW) {
        selectedIndex_ = (selectedIndex_ + 1) % items_.size();
        dirty();
    } else if (event.button == UP || event.type == ROTARY_CCW) {
        selectedIndex_ = (selectedIndex_ == 0 ? items_.size() - 1 : selectedIndex_ - 1);
        dirty();
    } else if (event.button == CENTER && event.type == PRESSED) {
        if (selectedIndex_ > items_.size()) return;
        auto& item = items_[selectedIndex_];
        if (item->getType() == MenuItemType::RANGE) adjustMode_ = true;
        else item->execute();
        dirty();
    }
}

void GenericMenuScreen::handleAdjustMode(InputEvent event) {
    static auto adjust = [this](int val) {
        items_[selectedIndex_]->adjust(val);
        dirty();
    };

    if (event.type == ROTARY_CW) adjust(10);
    else if (event.type == ROTARY_CCW) adjust(-10);
    else if (event.button == DOWN) adjust(-1);
    else if (event.button == UP) adjust(1);
    else if (event.button == CENTER && event.type == PRESSED) {
        adjustMode_ = false;
        dirty();
    }
}