#pragma once 
#include <Arduino.h>
#include <core/DeviceState.h>

enum CustomColors : uint8_t {
    // UI Basis-Farben (0-9)
    COLOR_PRIMARY= 0,
    COLOR_BG = 0,
    COLOR_BG_2 = 1,
    COLOR_BG_3 = 2,
    COLOR_TEXT_PRIMARY = 3,
    COLOR_TEXT_SECONDARY = 4,
    COLOR_ACCENT = 5,
    COLOR_SUCCESS = 6,
    COLOR_WARNING = 7,
    COLOR_ERROR = 8,
    COLOR_PURPLE = 12,
    COLOR_BLUE = 13,
    COLOR_BLACK = 11,

    COLOR_HIGHLIGHT = 14,
    
    // Fire/Heat Farben (10-19) - Gradient von schwarz zu weiß
    COLOR_HEAT_COLD = 13,
    COLOR_HEAT_LOW = 6,
    COLOR_HEAT_MEDIUM = 7,
    COLOR_HEAT_HIGH = 8,
    COLOR_HEAT_EXTREME = 12,
};

// Hilfsfunktion: RGB888 zu RGB565 Konvertierung
inline uint16_t rgb888to565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

const uint16_t heizbox_palette[16] = {
    rgb888to565(255, 105, 0),   //  0: COLOR_PRIMARY COLOR_BG
    rgb888to565(255, 143, 64),  //  1: COLOR_BG_2
    rgb888to565(255, 128, 38),  //  2: COLOR_BG_3

    rgb888to565(240, 240, 245), //  3: COLOR_TEXT_PRIMARY - Fast weiß
    rgb888to565(180, 180, 190), //  4: COLOR_TEXT_SECONDARY - Helles Grau

    rgb888to565(255, 105, 0),   //  5: COLOR_ACCENT - Orange/Rot Akzent
    rgb888to565(50, 200, 100),  //  6: COLOR_SUCCESS - Grün
    rgb888to565(255, 180, 50),  //  7: COLOR_WARNING - Gelb/Orange
    rgb888to565(255, 50, 50),   //  8: COLOR_ERROR - Rot
    rgb888to565(60, 60, 70),    //  9: 

    rgb888to565(0, 0, 0),                     // 10: 
    rgb888to565(0, 0, 0),       // 11: Schwarz
    rgb888to565(160, 32, 240),      // 12: Purple
    rgb888to565(45, 104, 196),      // 13: Blue
    rgb888to565(240, 240, 245),      // 14
    rgb888to565(230, 95, 0),     // 15
};

const uint16_t heizbox_palette_dark[16] = {
    rgb888to565(35, 0, 70),       //  0: COLOR_BG
    rgb888to565(66, 44, 100),  //  1: COLOR_BG_2
    rgb888to565(43, 22, 82),  //  2: COLOR_BG_3

    rgb888to565(234, 226, 243), //  3: COLOR_TEXT_PRIMARY - Fast weiß
    rgb888to565(180, 180, 190), //  4: COLOR_TEXT_SECONDARY - Helles Grau

    rgb888to565(255, 105, 0),   //  5: COLOR_ACCENT - Orange/Rot Akzent
    rgb888to565(71,213,166),  //  6: COLOR_SUCCESS - Grün
    rgb888to565(215,172,97),  //  7: COLOR_WARNING - Gelb/Orange
    rgb888to565(217,74,74),   //  8: COLOR_ERROR - Rot
    rgb888to565(60, 60, 70),    //  9: 

    rgb888to565(0, 0, 0),        // 10: 
    rgb888to565(0, 0, 0),       // 11: Schwarz
    rgb888to565(106,78,128),      // 12: Purple
    rgb888to565(64,119,209),      // 13: Blue
    rgb888to565(106,78,128),      // 14
    rgb888to565(230, 95, 0),     // 15
};

// Hilfsfunktionen für Farbinterpolation
class ColorUtils {
public:
    // Interpoliere zwischen zwei Palette-Indizes
    static uint8_t interpolatePalette(uint8_t startIdx, uint8_t endIdx, float t) {
        return startIdx + (uint8_t)((endIdx - startIdx) * t);
    }
    
    // Gib Farbe basierend auf Temperatur zurück (0-400°C)
    static uint8_t getTemperatureColor(float temp) {
        if (temp < 165) return COLOR_HEAT_COLD;
        else if (temp < 180) return COLOR_HEAT_LOW;
        else if (temp < 190) return COLOR_HEAT_MEDIUM;
        else if (temp < 200) return COLOR_HEAT_HIGH;
        else return COLOR_HEAT_EXTREME;
    }
    /*
    
    // Gib Fire-Farbe basierend auf Intensität zurück (0.0 - 1.0)
    static uint8_t getFireColor(float intensity) {
        if(intensity <= 0.0f) return COLOR_FIRE_BLACK;
        if(intensity >= 1.0f) return COLOR_FIRE_WHITE;
        
        // Interpoliere durch den Fire-Gradient
        uint8_t steps = COLOR_FIRE_WHITE - COLOR_FIRE_BLACK;
        return COLOR_FIRE_BLACK + (uint8_t)(steps * intensity);
    }
    // Gib Battery-Farbe basierend auf Prozent zurück
    static uint8_t getBatteryColor(uint8_t percent) {
        if(percent > 80) return COLOR_BATTERY_FULL;
        if(percent > 50) return COLOR_BATTERY_HIGH;
        if(percent > 20) return COLOR_BATTERY_MEDIUM;
        if(percent > 10) return COLOR_BATTERY_LOW;
        return COLOR_BATTERY_CRITICAL;
    }*/
};
