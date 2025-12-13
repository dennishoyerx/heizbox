#include "Menu.h"

void MenuManager::nextOption() {
    if (items_.empty()) return;
    currentIndex_ = (currentIndex_ + 1) % items_.size();
    items_[currentIndex_]->onEnter();
}
void MenuManager::prevOption() {
    if (items_.empty()) return;
    currentIndex_ = (currentIndex_ + items_.size() - 1) % items_.size();
    items_[currentIndex_]->onEnter();
}
void MenuManager::increment() {
    if (items_.empty()) return;
    items_[currentIndex_]->increment();
}
void MenuManager::decrement() {
    if (items_.empty()) return;
    items_[currentIndex_]->decrement();
}