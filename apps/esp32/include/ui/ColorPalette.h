#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

#include <Arduino.h>

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
    COLOR_BORDER = 9,
    COLOR_TEST = 15,
    COLOR_PURPLE = 12,
    COLOR_BLUE = 13,
    
    // Fire/Heat Farben (10-19) - Gradient von schwarz zu weiß
    COLOR_FIRE_BLACK = 10,
    COLOR_FIRE_DARK_RED = 11,
    COLOR_FIRE_RED = 12,
    COLOR_FIRE_ORANGE_RED = 13,
    COLOR_FIRE_ORANGE = 14,
    COLOR_FIRE_YELLOW_ORANGE = 15,
    COLOR_FIRE_YELLOW = 16,
    COLOR_FIRE_BRIGHT_YELLOW = 17,
    COLOR_FIRE_WHITE = 18,
    
    // Battery Level Indicator (60-69)
    COLOR_BATTERY_FULL = 60,
    COLOR_BATTERY_HIGH = 61,
    COLOR_BATTERY_MEDIUM = 62,
    COLOR_BATTERY_LOW = 63,
    COLOR_BATTERY_CRITICAL = 64,
    
    
    // Temperatur-Gradient (100-139) - Blau zu Rot
    COLOR_TEMP_COLD_START = 1,
    COLOR_TEMP_COLD_END = 1,
    COLOR_TEMP_WARM_START = 1,
    COLOR_TEMP_WARM_END = 1,
    COLOR_TEMP_HOT_START = 1,
    COLOR_TEMP_HOT_END = 1,
    COLOR_TEMP_EXTREME_START = 1,
    COLOR_TEMP_EXTREME_END = 1,
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
    rgb888to565(60, 60, 70),    //  9: COLOR_BORDER - Dunkles Grau

    0xFB40,                     // 10: Reserve
    rgb888to565(0, 0, 0),       // 11: Schwarz
    rgb888to565(160, 32, 240),      // 12: Purple
    rgb888to565(45, 104, 196),      // 13: Blue
    rgb888to565(60, 5, 0),      // 14
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
    static uint8_t getTemperatureColor(float tempC) {
        if(tempC < 0) return COLOR_TEMP_COLD_START;
        if(tempC > 400) return COLOR_TEMP_EXTREME_END;
        
        if(tempC < 100) {
            // 0-100°C: Blau-Bereich
            float t = tempC / 100.0f;
            return interpolatePalette(COLOR_TEMP_COLD_START, COLOR_TEMP_COLD_END, t);
        } else if(tempC < 200) {
            // 100-200°C: Grün-Bereich
            float t = (tempC - 100) / 100.0f;
            return interpolatePalette(COLOR_TEMP_WARM_START, COLOR_TEMP_WARM_END, t);
        } else if(tempC < 300) {
            // 200-300°C: Gelb-Orange-Bereich
            float t = (tempC - 200) / 100.0f;
            return interpolatePalette(COLOR_TEMP_HOT_START, COLOR_TEMP_HOT_END, t);
        } else {
            // 300-400°C: Rot-Weiß-Bereich
            float t = (tempC - 300) / 100.0f;
            return interpolatePalette(COLOR_TEMP_EXTREME_START, COLOR_TEMP_EXTREME_END, t);
        }
    }
    
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
    }
};

#endif // COLOR_PALETTE_H

