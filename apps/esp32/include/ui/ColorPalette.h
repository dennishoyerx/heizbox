// ColorPalette.h - Definiere deine eigene Farbpalette
#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

#include <Arduino.h>

// Deine benutzerdefinierten Farb-Indizes (0-255)
// Diese kannst du wie normale Farben verwenden
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
    
    // Fire/Heat Farben (10-49) - Gradient von schwarz zu weiß
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
    COLOR_TEMP_COLD_START = 100,
    COLOR_TEMP_COLD_END = 109,
    COLOR_TEMP_WARM_START = 110,
    COLOR_TEMP_WARM_END = 119,
    COLOR_TEMP_HOT_START = 120,
    COLOR_TEMP_HOT_END = 129,
    COLOR_TEMP_EXTREME_START = 130,
    COLOR_TEMP_EXTREME_END = 139,
    
    // Reserve für weitere Farben (140-255)
};

// Hilfsfunktion: RGB888 zu RGB565 Konvertierung
inline uint16_t rgb888to565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Die komplette Palette mit 256 Farben (RGB565 Format)
const uint16_t heizbox_palette[256] PROGMEM = {
    // UI Basis-Farben (0-9)
    rgb888to565(15, 15, 20),      // 0: COLOR_BG_DARK - Fast schwarz mit Blaustich
    rgb888to565(30, 30, 35),      // 1: COLOR_BG_MEDIUM
    rgb888to565(240, 240, 245),   // 2: COLOR_TEXT_PRIMARY - Fast weiß
    rgb888to565(180, 180, 190),   // 3: COLOR_TEXT_SECONDARY - Helles Grau
    rgb888to565(255, 100, 50),    // 4: COLOR_ACCENT - Orange/Rot Akzent
    rgb888to565(50, 200, 100),    // 5: COLOR_SUCCESS - Grün
    rgb888to565(255, 180, 50),    // 6: COLOR_WARNING - Gelb/Orange
    rgb888to565(255, 50, 50),     // 7: COLOR_ERROR - Rot
    rgb888to565(60, 60, 70),      // 8: COLOR_BORDER - Dunkles Grau
    rgb888to565(0, 0, 0),         // 9: Reserve
    
    // Fire/Heat Gradient (10-49) - 40 Farbstufen für Flammen-Animation
    rgb888to565(0, 0, 0),         // 10: Schwarz
    rgb888to565(20, 0, 0),        // 11: Sehr dunkles Rot
    rgb888to565(40, 0, 0),        // 12
    rgb888to565(60, 5, 0),        // 13
    rgb888to565(80, 10, 0),       // 14
    rgb888to565(100, 15, 0),      // 15
    rgb888to565(120, 20, 0),      // 16
    rgb888to565(140, 25, 0),      // 17
    rgb888to565(160, 30, 0),      // 18
    rgb888to565(180, 40, 0),      // 19
    rgb888to565(200, 50, 0),      // 20: Rot
    rgb888to565(220, 60, 0),      // 21
    rgb888to565(240, 70, 0),      // 22
    rgb888to565(255, 80, 0),      // 23
    rgb888to565(255, 100, 0),     // 24: Orange-Rot
    rgb888to565(255, 120, 0),     // 25
    rgb888to565(255, 140, 0),     // 26
    rgb888to565(255, 160, 0),     // 27: Orange
    rgb888to565(255, 180, 0),     // 28
    rgb888to565(255, 200, 0),     // 29
    rgb888to565(255, 220, 0),     // 30: Gelb-Orange
    rgb888to565(255, 240, 0),     // 31
    rgb888to565(255, 255, 0),     // 32: Gelb
    rgb888to565(255, 255, 40),    // 33
    rgb888to565(255, 255, 80),    // 34
    rgb888to565(255, 255, 120),   // 35
    rgb888to565(255, 255, 160),   // 36
    rgb888to565(255, 255, 200),   // 37
    rgb888to565(255, 255, 240),   // 38: Fast Weiß
    rgb888to565(255, 255, 255),   // 39: Weiß
    // 40-49: Reserve für weitere Fire-Farben
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0),
    
    // Status-LEDs (50-59)
    rgb888to565(30, 30, 30),      // 50: LED aus (dunkelgrau)
    rgb888to565(0, 255, 100),     // 51: Grün (online)
    rgb888to565(255, 220, 0),     // 52: Gelb (warning)
    rgb888to565(255, 50, 50),     // 53: Rot (error)
    rgb888to565(0, 150, 255),     // 54: Blau
    rgb888to565(200, 0, 255),     // 55: Lila
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), 
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    
    // Battery Levels (60-69)
    rgb888to565(0, 255, 100),     // 60: Voll (Grün)
    rgb888to565(100, 255, 50),    // 61: Hoch (Hellgrün)
    rgb888to565(255, 200, 0),     // 62: Mittel (Gelb)
    rgb888to565(255, 100, 0),     // 63: Niedrig (Orange)
    rgb888to565(255, 0, 0),       // 64: Kritisch (Rot)
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), 
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), 
    rgb888to565(0, 0, 0),
    
    // Graustufen (70-99) - 30 Stufen
    rgb888to565(26, 26, 26),      // 70: 10%
    rgb888to565(51, 51, 51),      // 71: 20%
    rgb888to565(77, 77, 77),      // 72: 30%
    rgb888to565(102, 102, 102),   // 73: 40%
    rgb888to565(128, 128, 128),   // 74: 50%
    rgb888to565(153, 153, 153),   // 75: 60%
    rgb888to565(179, 179, 179),   // 76: 70%
    rgb888to565(204, 204, 204),   // 77: 80%
    rgb888to565(230, 230, 230),   // 78: 90%
    // 79-99: Weitere Graustufen und Reserve
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    
    // Temperatur-Gradient (100-139) - Blau → Cyan → Grün → Gelb → Rot
    // Kalt (100-109): Blautöne
    rgb888to565(0, 50, 150),      // 100: Kalt Start (Dunkelblau)
    rgb888to565(0, 80, 180),
    rgb888to565(0, 110, 210),
    rgb888to565(0, 140, 240),
    rgb888to565(30, 170, 255),
    rgb888to565(60, 200, 255),
    rgb888to565(90, 220, 255),
    rgb888to565(120, 235, 255),
    rgb888to565(150, 245, 255),
    rgb888to565(180, 250, 255),   // 109: Kalt Ende (Hellblau)
    
    // Warm (110-119): Cyan → Grün
    rgb888to565(150, 255, 230),   // 110
    rgb888to565(120, 255, 200),
    rgb888to565(90, 255, 170),
    rgb888to565(60, 255, 140),
    rgb888to565(50, 255, 100),
    rgb888to565(60, 240, 70),
    rgb888to565(80, 220, 50),
    rgb888to565(100, 200, 40),
    rgb888to565(130, 180, 30),
    rgb888to565(160, 160, 20),    // 119
    
    // Heiß (120-129): Gelb → Orange
    rgb888to565(200, 200, 0),     // 120
    rgb888to565(220, 190, 0),
    rgb888to565(240, 180, 0),
    rgb888to565(255, 170, 0),
    rgb888to565(255, 150, 0),
    rgb888to565(255, 130, 0),
    rgb888to565(255, 110, 0),
    rgb888to565(255, 90, 0),
    rgb888to565(255, 70, 0),
    rgb888to565(255, 50, 0),      // 129
    
    // Extrem (130-139): Rot → Weiß
    rgb888to565(255, 30, 0),      // 130
    rgb888to565(255, 20, 20),
    rgb888to565(255, 30, 40),
    rgb888to565(255, 50, 60),
    rgb888to565(255, 80, 80),
    rgb888to565(255, 110, 110),
    rgb888to565(255, 150, 150),
    rgb888to565(255, 190, 190),
    rgb888to565(255, 220, 220),
    rgb888to565(255, 240, 240),   // 139: Extrem Ende
    
    // Rest mit Schwarz auffüllen (140-255)
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0), rgb888to565(0, 0, 0),
    rgb888to565(0, 0, 0), rgb888to565(0, 0, 0)
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

