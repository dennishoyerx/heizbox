#pragma once
#include <Arduino.h>

namespace Audio {

// ===== Notes =====
enum : uint16_t {
    REST = 0,
    C4 = 262, D4 = 294, E4 = 330, F4 = 349, G4 = 392,
    A4 = 440, B4 = 494,
    C5 = 523, D5 = 587, E5 = 659, F5 = 698, G5 = 784,
    A5 = 880, B5 = 988,
    C6 = 1047, E6 = 1319
};

// ===== Core =====
void init();
void setVolume(uint8_t percent);   // 0..100
void mute(bool on);

// ===== Sounds =====
void beepMenu();
void beepHeatCycleSwitch();
void beepHeatStart();
void beepHeatFinish();
void beepError();
void beepSuccess();
void beepStartup();
void beepClick();
void beepWarning();
void beepBatteryLow();

void playSuperMario();
void playTetris();

void beepCycleUp();
void beepCycleDown();


}
