#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

#include <Arduino.h>

enum CustomColors : uint8_t {
    // UI Basis-Farben (0-9)
    COLOR_BG_DARK = 0,
    COLOR_BG_MEDIUM = 1,
    COLOR_TEXT_PRIMARY = 2,
    COLOR_TEXT_SECONDARY = 3,
    COLOR_ACCENT = 4,
    COLOR_SUCCESS = 5,
    COLOR_WARNING = 6,
    COLOR_ERROR = 7,
    COLOR_BORDER = 8,
    COLOR_TEST = 9,
    
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

    // Status-LEDs (50-59)
    COLOR_LED_OFF = 50,
    COLOR_LED_GREEN = 51,
    COLOR_LED_YELLOW = 52,
    COLOR_LED_RED = 53,
    
    // Battery Level Indicator (60-69)
    COLOR_BATTERY_FULL = 60,
    COLOR_BATTERY_HIGH = 61,
    COLOR_BATTERY_MEDIUM = 62,
    COLOR_BATTERY_LOW = 63,
    COLOR_BATTERY_CRITICAL = 64,
    
    // Graustufen (70-99) - für Schatten, Verläufe etc.
    COLOR_GRAY_10 = 70,   // Sehr dunkel
    COLOR_GRAY_20 = 71,
    COLOR_GRAY_30 = 72,
    COLOR_GRAY_40 = 73,
    COLOR_GRAY_50 = 74,   // Mittelgrau
    COLOR_GRAY_60 = 75,
    COLOR_GRAY_70 = 76,
    COLOR_GRAY_80 = 77,
    COLOR_GRAY_90 = 78,   // Sehr hell
    
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

// Die komplette Palette mit 256 Farben (RGB565 Format)
const uint16_t heizbox_palette[16] = {
    // UI Basis-Farben (0-9)
    0xFB40,    // 0: COLOR_BG_DARK
    rgb888to565(30, 30, 35),      // 1: COLOR_BG_MEDIUM
    rgb888to565(240, 240, 245),   // 2: COLOR_TEXT_PRIMARY - Fast weiß
    rgb888to565(180, 180, 190),   // 3: COLOR_TEXT_SECONDARY - Helles Grau
    0xFB40,    // 4: COLOR_ACCENT - Orange/Rot Akzent
    rgb888to565(50, 200, 100),    // 5: COLOR_SUCCESS - Grün
    rgb888to565(255, 180, 50),    // 6: COLOR_WARNING - Gelb/Orange
    rgb888to565(255, 50, 50),     // 7: COLOR_ERROR - Rot
    rgb888to565(60, 60, 70),      // 8: COLOR_BORDER - Dunkles Grau
    0xFB40,         // 9: Reserve
    
    // Fire/Heat Gradient (10-49) - 40 Farbstufen für Flammen-Animation
    rgb888to565(0, 0, 0),         // 10: Schwarz
    rgb888to565(20, 0, 0),        // 11: Sehr dunkles Rot
    rgb888to565(40, 0, 0),        // 12
    rgb888to565(60, 5, 0),        // 13
    rgb888to565(80, 10, 0),       // 14
    rgb888to565(100, 15, 0),      // 15
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

