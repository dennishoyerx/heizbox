#pragma once

#include <ui/base/SurfaceFactory.h>
#include "Component.h"
#include <unordered_map>
#include <string>

class Components {
public:
    Components(SurfaceFactory* factory): _surfaceFactory(factory) {

    };

    void add(std::string key, Component comp) {
        components[key] = comp;
    };

    void renderComponent(const std::string& key) {
        auto it = components.find(key);
        if (it != components.end()) {
            // it->second ist die Comp-Instanz
            // it->second.renderExecute(); // Dies könnte die beabsichtigte Aktion sein
        }
    };

private: 
    SurfaceFactory* _surfaceFactory;
    std::unordered_map<std::string, Component> components;
};

