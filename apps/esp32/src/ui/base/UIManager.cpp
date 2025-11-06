#include "UIManager.h"
#include <utility>  // für std::move

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

UICText* UIManager::Text(const std::string& id, const std::string& text) {
    if (auto existing = getComponent(id)) {
        if (auto textComp = dynamic_cast<UICText*>(existing)) {
            textComp->setText(text);
            return textComp;
        }
    }
    auto component = std::make_unique<UICText>();
    component->setText(text);
    auto ptr = component.get();
    components[id] = std::move(component);
    return ptr;
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
