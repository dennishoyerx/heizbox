 #include "Audio.h"
#include "Config.h"
#include <Preferences.h>


Speaker::Driver::Driver(int pin_, int channel_, int resolution_) {
    pin = pin_;
    channel = channel_;
    resolution = resolution_;
}

void Speaker::Driver::init() {
    pinMode(pin, OUTPUT);
    ledcSetup(channel, 2000, resolution);
    ledcAttachPin(pin, channel);
}

void Speaker::Driver::setVolume(uint8_t volumePercent) {
    masterVolume = constrain(volumePercent, 0, 100);
}

void Speaker::Driver::setMute(bool mute) {
    muted = mute;
}



namespace Audio {
    const int CH = 0;
    const int RESOLUTION = 8;
    const uint8_t MAX_VOLUME = 255;
    const uint8_t DEFAULT_VOLUME = 200; // Erhöht von 100
    
    // Globale Lautstärke-Einstellungen
    static uint8_t masterVolume = 70;  // 0-100%
    static bool audioMuted = false;
    
    // Hilfsfunktion: Berechnet tatsächlichen PWM-Wert
    static uint8_t calculatePWM(uint8_t requestedVolume) {
        if (audioMuted) return 0;
        
        // requestedVolume (0-255) * masterVolume (0-100%) = finaler PWM-Wert
        uint16_t pwm = (requestedVolume * masterVolume) / 100;
        return constrain(pwm, 0, 255);
    }

    void init() {
        ledcSetup(CH, 2000, RESOLUTION);
        ledcAttachPin(HardwareConfig::SPEAKER_PIN, CH);
    }

    // ====== Lautstärke-Kontrolle ======
    
    void setVolume(uint8_t volumePercent) {
        masterVolume = constrain(volumePercent, 0, 100);
    }
    
    uint8_t getVolume() {
        return masterVolume;
    }
    
    void mute() {
        audioMuted = true;
    }
    
    void unmute() {
        audioMuted = false;
    }
    
    bool isMuted() {
        return audioMuted;
    }

    void playTone(uint32_t freq, uint16_t durationMs, uint8_t volume, bool fadeOut) {
        if (freq == 0) {
            delay(durationMs);
            return;
        }
        
        ledcWriteTone(CH, freq);
        
        if (fadeOut && durationMs > 50) {
            uint16_t fadeTime = min(durationMs / 2, 100);
            ledcWrite(CH, volume);
            delay(durationMs - fadeTime);
            
            // Sanftes Fade-Out
            for (int v = volume; v > 0; v -= max(1, volume / 10)) {
                ledcWrite(CH, v);
                delay(fadeTime / 10);
            }
        } else {
            ledcWrite(CH, volume);
            delay(durationMs);
        }
        
        // Komplett ausschalten
        ledcWrite(CH, 0);
        ledcWriteTone(CH, 0);  // Frequenz auch auf 0
        ledcDetachPin(HardwareConfig::SPEAKER_PIN);  // Pin detachen
        pinMode(HardwareConfig::SPEAKER_PIN, OUTPUT);
        digitalWrite(HardwareConfig::SPEAKER_PIN, LOW);  // Pin auf LOW
    }

    void playMelody(const Note notes[], const uint16_t durations[], uint8_t length, uint8_t volume) {
        if (audioMuted) return;
        
        // Pin vor Melodie wieder attachen
        ledcAttachPin(HardwareConfig::SPEAKER_PIN, CH);
        
        uint8_t actualVolume = calculatePWM(volume);
        
        for (uint8_t i = 0; i < length; i++) {
            if (notes[i] == REST) {
                delay(durations[i]);
            } else {
                ledcWriteTone(CH, notes[i]);
                ledcWrite(CH, actualVolume);
                delay(durations[i]);
                ledcWrite(CH, 0);
            }
            delay(20); // Kurze Pause zwischen Noten
        }
        
        // Nach Melodie komplett ausschalten
        ledcWrite(CH, 0);
        ledcWriteTone(CH, 0);
        ledcDetachPin(HardwareConfig::SPEAKER_PIN);
        pinMode(HardwareConfig::SPEAKER_PIN, OUTPUT);
        digitalWrite(HardwareConfig::SPEAKER_PIN, LOW);
    }

    void chirp(uint32_t startFreq, uint32_t endFreq, uint16_t durationMs, uint8_t volume) {
        if (audioMuted) return;
        
        ledcAttachPin(HardwareConfig::SPEAKER_PIN, CH);
        
        uint8_t actualVolume = calculatePWM(volume);
        
        const uint8_t steps = 20;
        const uint16_t stepDuration = durationMs / steps;
        const int32_t freqStep = (endFreq - startFreq) / steps;
        
        for (uint8_t i = 0; i < steps; i++) {
            uint32_t freq = startFreq + (freqStep * i);
            ledcWriteTone(CH, freq);
            ledcWrite(CH, actualVolume);
            delay(stepDuration);
        }
        
        ledcWrite(CH, 0);
        ledcWriteTone(CH, 0);
        ledcDetachPin(HardwareConfig::SPEAKER_PIN);
        pinMode(HardwareConfig::SPEAKER_PIN, OUTPUT);
        digitalWrite(HardwareConfig::SPEAKER_PIN, LOW);
    }

    void siren(uint8_t cycles, uint16_t cycleMs, uint8_t volume) {
        for (uint8_t i = 0; i < cycles; i++) {
            chirp(800, 1200, cycleMs / 2, volume);
            chirp(1200, 800, cycleMs / 2, volume);
        }
    }

    void robotSound() {
        static const Note melody[] PROGMEM = {C5, E5, G5, C6, G5, E5};
        static const uint16_t durations[] PROGMEM = {50, 50, 50, 100, 50, 80};
        playMelody(melody, durations, 6, 100);
    }

    // ====== Sound-Effekte ======

    void beepMenu() {
        playTone(1800, 40, 60);
        delay(20);
        playTone(2200, 30, 40);
    }

    void beepHeatStart() {
        static const Note melody[] PROGMEM = {C4, E4, G4, C5};
        static const uint16_t durations[] PROGMEM = {60, 60, 80, 150};
        playMelody(melody, durations, 4, 120);
    }

    void beepHeatFinish() {
        static const Note melody[] PROGMEM = {E5, G5, C6, REST, C6, E6};
        static const uint16_t durations[] PROGMEM = {100, 100, 200, 50, 80, 150};
        playMelody(melody, durations, 6, 100);
    }

    void beepError() {
        playTone(400, 150, 150);
        delay(50);
        playTone(300, 200, 150);
    }

    void beepSuccess() {
        playTone(C5, 60, 80);
        delay(30);
        playTone(E5, 60, 80);
        delay(30);
        playTone(G5, 120, 100, true);
    }

    void beepStartup() {
        chirp(500, 2000, 400, 100);
        delay(100);
        playTone(C6, 150, 120, true);
    }

    void beepClick() {
        playTone(2500, 25, 50);
    }

    void beepWarning() {
        for (uint8_t i = 0; i < 3; i++) {
            playTone(1000, 100, 150);
            delay(100);
        }
    }

    void beepBatteryLow() {
        playTone(800, 200, 100);
        delay(150);
        playTone(600, 300, 80);
        delay(150);
        playTone(400, 400, 60, true);
    }

    // ====== Easter Eggs ======

    void playSuperMario() {
        static const Note melody[] PROGMEM = {E5, E5, REST, E5, REST, C5, E5, REST, G5};
        static const uint16_t durations[] PROGMEM = {100, 100, 100, 100, 100, 100, 100, 100, 200};
        playMelody(melody, durations, 9, 90);
    }

    void playTetris() {
        static const Note melody[] PROGMEM = {E5, B4, C5, D5, C5, B4, A4, A4, C5, E5, D5, C5, B4};
        static const uint16_t durations[] PROGMEM = {100, 50, 50, 100, 50, 50, 100, 50, 50, 100, 50, 50, 100};
        playMelody(melody, durations, 13, 80);
    }
}