// include/ScreenRegistry.h
#ifndef SCREENREGISTRY_H
#define SCREENREGISTRY_H

#include "Screen.h"
#include "ScreenType.h"
#include <unordered_map>
#include <memory>

// ============================================================================ 
// Screen Registry - Zentrales Screen-Management
// ============================================================================ 

class ScreenRegistry {
public:
    static ScreenRegistry& instance() {
        static ScreenRegistry registry;
        return registry;
    }
    
    // Screen registrieren
    void registerScreen(ScreenType type, Screen* screen) {
        screens_[type] = screen;
    }
    
    // Screen abrufen
    Screen* getScreen(ScreenType type) {
        auto it = screens_.find(type);
        return (it != screens_.end()) ? it->second : nullptr;
    }
    
    // Screen existiert?
    bool hasScreen(ScreenType type) const {
        return screens_.find(type) != screens_.end();
    }
    
    // Cleanup (optional)
    void clear() {
        screens_.clear();
    }

private:
    ScreenRegistry() = default;
    std::unordered_map<ScreenType, Screen*> screens_;
};

// Helper-Makro für einfache Registrierung
#define REGISTER_SCREEN(type, screen) \
    ScreenRegistry::instance().registerScreen(type, &screen)

// ============================================================================ 
// Screen Transition Helper
// ============================================================================ 

struct ScreenTransitionConfig {
    ScreenType from;
    ScreenType to;
    ScreenTransition transition;
    
    // Optional: Bedingungen
    std::function<bool()> condition;
};

class ScreenNavigator {
public:
    static void navigateTo(ScreenManager* manager, ScreenType type, 
                          ScreenTransition transition = ScreenTransition::FADE) {
        Screen* screen = ScreenRegistry::instance().getScreen(type);
        if (screen && manager) {
            manager->setScreen(screen, transition);
        }
    }
    
    static void back(ScreenManager* manager, ScreenType defaultType = ScreenType::FIRE) {
        // Könnte History-Stack implementieren
        navigateTo(manager, defaultType);
    }
};

// Helper-Makro für Navigation
#define NAVIGATE_TO(manager, type) \
    ScreenNavigator::navigateTo(manager, type)

#define NAVIGATE_TO_WITH_TRANSITION(manager, type, transition) \
    ScreenNavigator::navigateTo(manager, type, transition)

#define NAVIGATE_BACK(manager) \
    ScreenNavigator::back(manager)

#endif // SCREENREGISTRY_H
