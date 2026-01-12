#pragma once
#include <Arduino.h>

/*
enum Mode {
    TEMP,
    PRESET
};
struct HeaterControls {
    void on();
    void off();
    void pause();
    void setPower(uint8_t percent);
};

enum HeaterActions {
    START,
    STOP,
    PAUSE,
    POWER
};

class HeatMode {
public:
    HeatMode();
    void update();
    void setMode(Mode mode) { currentMode = mode; }
    Mode getMode() { return currentMode; }

private:
    Mode currentMode;
    HeaterControls heater;
};


class IHeatStrategy {
public:
    IHeatStrategy(HeaterControls heater);
    virtual void start();
    virtual void stopped();
    virtual void update();
protected:
    HeaterControls heater;
};

class TempControl : public IHeatStrategy {
public:
    TempControl();
    void update();
private:
    uint16_t limit;
};

enum TempPresets {
    FLAVOR,
    BALANCED,
    EXTRACTION,
    FULL,
};

struct TempPreset {
    uint16_t temp;
    String name;
};

TempPreset TempPresets[] = {
    { 170, "FLAVOR" },
    { 180, "BALANCED" },
    { 200, "EXTRACTION" },
    { 220, "FULL" },
};

class PresetC {
public:
    PresetC();
};*/