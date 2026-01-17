#include "ui/base/GenericMenuScreen.h"
#include "hardware/input/InputManager.h"
#include "ui/base/ScreenManager.h" 
#include "ui/ColorPalette.h"
#include <algorithm> // For std::min


GenericMenuScreen::GenericMenuScreen(const char* title, std::vector<std::unique_ptr<MenuItem>> items) : 
        title_(title), items_(std::move(items)), selectedIndex_(0), adjustMode_(false) {
            selectedIndex_ = 0;
        while (selectedIndex_ < items_.size() && items_[selectedIndex_]->getType() == MenuItemType::HEADLINE) {
            selectedIndex_++;
        }

        // Fallback, falls alle HEADLINEs
        if (selectedIndex_ >= items_.size()) selectedIndex_ = 0;
    }

// Implementations of methods declared in GenericMenuScreen.h
void GenericMenuScreen::draw() {
    _ui->clear();    
    _ui->withSurface(280, 240, 0, 0, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);

    //centerText(s.sprite, 10, title_, COLOR_TEXT_PRIMARY, 2);
    
    // Items (with scrolling support)
    const int itemsPerPage = 5;
    const int startIdx = (selectedIndex_ / itemsPerPage) * itemsPerPage;
    const int endIdx = std::min(startIdx + itemsPerPage, static_cast<int>(items_.size()));
    
    for (int i = startIdx; i < endIdx; i++) {
        const int displayIdx = i - startIdx;
        const int16_t y = 50 + displayIdx * 30;
        
        const auto& item = items_[i];
        const bool isSelected = (i == selectedIndex_);
        const uint8_t color = isSelected ? COLOR_ACCENT : COLOR_TEXT_PRIMARY;
        s.sprite->setFreeFont(&FreeSans12pt7b);
        s.sprite->setTextColor(COLOR_TEXT_PRIMARY);
        
        // Selection indicator
        if (isSelected) {
        s.sprite->setCursor(10, y);
        s.sprite->print(adjustMode_ ? ">" : "*");
            //s.sprite->drawText(10, y, adjustMode_ ? ">" : "*", color, 2);
        }
        
        // Item title
        //s.sprite->drawText(30, y, item->getTitle(), color, 2);
        s.sprite->setCursor(30, y);
        s.sprite->print(item->getTitle());
        
        // Item value (if any)
        const char* value = item->getValue();
        if (value) {
            const int16_t valueX = 200;
            //s.sprite->drawString(valueX, y, value, color, 2);
        s.sprite->setCursor(valueX, y);
        s.sprite->print(value);
        }
    }
    
    // Footer
    const char* footer = adjustMode_ 
        ? "L/R: Adjust  OK: Done"
        : "OK: Select  HOLD L: Back";
//    s.sprite->drawText(10, 210, footer, COLOR_TEXT_SECONDARY, 1);
        s.sprite->setFreeFont(&FreeSans9pt7b);
        s.sprite->setCursor(10, 210);
        s.sprite->print(footer);
    });
}

void GenericMenuScreen::handleInput(InputEvent event) {
    if (event.type != PRESSED && event.type != HOLD && event.type != HOLD_ONCE && event.type != ROTARY_CCW && event.type != ROTARY_CW) return;
    
    if (adjustMode_) {
        handleAdjustMode(event);
    } else {
        handleNavigationMode(event);
    }
}

void GenericMenuScreen::handleNavigationMode(InputEvent event) {
    switch (event.button) {
                case DOWN:
                case ROTARY_CW:
            do {
                selectedIndex_ = (selectedIndex_ + 1) % items_.size();
            } while (items_[selectedIndex_]->getType() == MenuItemType::HEADLINE);
            dirty();
            break;

        case UP:
        case ROTARY_CCW:
            do {
                selectedIndex_ = (selectedIndex_ == 0 ? items_.size() - 1 : selectedIndex_ - 1);
            } while (items_[selectedIndex_]->getType() == MenuItemType::HEADLINE);
            dirty();
            break;
            
        case CENTER:
            if (selectedIndex_ < items_.size()) {
                auto& item = items_[selectedIndex_];
                
                if (item->getType() == MenuItemType::RANGE) {
                    adjustMode_ = true;
                } else {
                    item->execute();
                }
                dirty();
            }
            break;
            
        default:
            break;
    }
}

void GenericMenuScreen::handleAdjustMode(InputEvent event) {
    if (event.type == ROTARY_CW) {
        items_[selectedIndex_]->adjust(10);
        dirty();
        return;
    } else if (event.type == ROTARY_CCW) {
        items_[selectedIndex_]->adjust(-10);
        dirty();
        return;
    } 
    switch (event.button) {
        case LEFT:
        case DOWN:
            items_[selectedIndex_]->adjust(-1);
            dirty();
            break;
            
        case RIGHT:
        case UP:
            items_[selectedIndex_]->adjust(1);
            dirty();
            break;
            
        case CENTER:
            adjustMode_ = false;
            dirty();
            break;
            
        default:
            break;
    }
}