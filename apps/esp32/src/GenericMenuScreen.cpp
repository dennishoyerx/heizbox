// src/GenericMenuScreen.cpp
#include "MenuBuilder.h"
#include "InputManager.h" // For InputEvent and its enums
#include "ScreenManager.h" // For ScreenTransition

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
