#include "UIManager.h"
#include "hardware/display/DisplayDriver.h"
#include <utility>  // f�r std::move

// ---------- UIBaseComponent ----------
void UIBaseComponent::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
}

// ---------- UICText ----------
void UICText::setText(const std::string& text) {
    this->text = text;
}

void UICText::draw(DisplayDriver& display) {
    display.drawText(x, y, text.c_str(), color, size);
}

// ---------- UIManager ----------
UICText UIManager::Text(const std::string& text, int x, int y) {
    UICText component;
    component.setText(text);
    component.setPosition(x, y);
    return component;
}

void UIManager::registerUIComponent(const std::string& id, std::unique_ptr<UIBaseComponent> component) {
    components[id] = std::move(component);
}

void UIManager::unregisterUIComponent(const std::string& id) {
    components.erase(id);
}

UIBaseComponent* UIManager::getComponent(const std::string& id) {
    auto it = components.find(id);
    return (it != components.end()) ? it->second.get() : nullptr;
}
