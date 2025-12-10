#ifndef SCREENTRANSITION_H
#define SCREENTRANSITION_H

#include <cstdint>

enum class ScreenType : uint8_t {
    STARTUP,
    FIRE,
    STATS,
    TIMEZONE,
    SCREENSAVER,
    OTA_UPDATE,
    HIDDEN_MODE,
    MAIN_MENU,
    HEAT_MENU,
    DEBUG_MENU,
};

// Transition-Effekte für Screen-Wechsel
enum class ScreenTransition : uint8_t {
    NONE,           // Direkt wechseln
    FADE,           // Fade out/in (falls genug RAM)
    SLIDE_LEFT,     // Slide-Animation (zukünftig)
    SLIDE_RIGHT
};


#endif