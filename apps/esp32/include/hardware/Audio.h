// Audio.h
#pragma once
#include <Arduino.h>

namespace Audio {
    // Musikalische Noten in Hz (C4-C6)
    enum Note : uint32_t {
        C4 = 262, D4 = 294, E4 = 330, F4 = 349, G4 = 392, A4 = 440, B4 = 494,
        C5 = 523, D5 = 587, E5 = 659, F5 = 698, G5 = 784, A5 = 880, B5 = 988,
        C6 = 1047, D6 = 1175, E6 = 1319, F6 = 1397, G6 = 1568, A6 = 1760, B6 = 1976,
        REST = 0
    };

    void init();
    
    // Grundlegende Ton-Funktionen
    void playTone(uint32_t freq, uint16_t durationMs, uint8_t volume = 100, bool fadeOut = false);
    void playMelody(const Note notes[], const uint16_t durations[], uint8_t length, uint8_t volume = 80);
    
    // Effekte
    void chirp(uint32_t startFreq, uint32_t endFreq, uint16_t durationMs, uint8_t volume = 100);
    void siren(uint8_t cycles = 2, uint16_t cycleMs = 300, uint8_t volume = 120);
    void robotSound();
    
    // Standard Sound-Effekte
    void beepMenu();
    void beepHeatStart();
    void beepHeatFinish();
    void beepError();
    void beepSuccess();
    void beepStartup();
    void beepClick();
    void beepWarning();
    void beepBatteryLow();
    
    // Easter Eggs
    void playSuperMario();
    void playTetris();

    void setVolume(uint8_t volumePercent);
    uint8_t getVolume();
    void mute();
    void unmute();
    bool isMuted();
}