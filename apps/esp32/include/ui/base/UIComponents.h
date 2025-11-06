#ifndef UICOMPONENTS_H
#define UICOMPONENTS_H

#include <unordered_map>
#include <string>
#include "ui/base/UIComponent.h"

class DisplayDriver;

//template<typename T>
class UIComponents {
public:
    void addComponent(const std::string& id, std::unique_ptr<UIComponent> c) {
        components_[id] = std::move(c);
    }

    void removeComponent(const std::string& id) {
        components_.erase(id);
    }

    UIComponent* getComponent(const std::string& id) const {
        auto it = components_.find(id);
        return it != components_.end() ? it->second.get() : nullptr;
    }

    void forEach(const std::function<void(UIComponent&, DisplayDriver&)>& fn, DisplayDriver& display) {
        for (auto it = components_.begin(); it != components_.end(); ++it) {
            fn(*it->second, display);
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<UIComponent>> components_;
};

#endif
