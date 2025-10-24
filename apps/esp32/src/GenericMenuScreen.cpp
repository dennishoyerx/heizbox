// src/GenericMenuScreen.cpp
#include "MenuBuilder.h"
#include "InputManager.h" // For InputEvent and its enums
#include "ScreenManager.h" // For ScreenTransition
#include "DisplayManager.h"

namespace {
    void centerText(DisplayManager& display, int16_t y, const char* text, uint16_t color, uint8_t size) {
        int16_t x = (display.getTFTWidth() - display.getTextWidth(text, size)) / 2;
        display.drawText(x, y, text, color, size);
    }
}

void GenericMenuScreen::draw(DisplayManager& display) {
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

void GenericMenuScreen::handleInput(InputEvent event) {
    if (event.type != PRESS) return;
    
    if (adjustMode_) {
        handleAdjustMode(event);
    } else {
        handleNavigationMode(event);
    }
}

void GenericMenuScreen::handleNavigationMode(InputEvent event) {
    switch (event.button) {
        case UP:
            selectedIndex_ = (selectedIndex_ == 0) 
                ? items_.size() - 1 
                : selectedIndex_ - 1;
            markDirty();
            break;
            
        case DOWN:
            selectedIndex_ = (selectedIndex_ + 1) % items_.size();
            markDirty();
            break;
            
        case CENTER:
            if (selectedIndex_ < items_.size()) {
                auto& item = items_[selectedIndex_];
                
                if (item->getType() == MenuItemType::RANGE) {
                    adjustMode_ = true;
                } else {
                    item->execute();
                }
                markDirty();
            }
            break;
            
        default:
            break;
    }
}

void GenericMenuScreen::handleAdjustMode(InputEvent event) {
    switch (event.button) {
        case LEFT:
            items_[selectedIndex_]->adjust(-1);
            markDirty();
            break;
            
        case RIGHT:
            items_[selectedIndex_]->adjust(1);
            markDirty();
            break;
            
        case CENTER:
            adjustMode_ = false;
            markDirty();
            break;
            
        default:
            break;
    }
}