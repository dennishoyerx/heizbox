#pragma once

#include "ui/base/Screen.h"
#include "ui/base/UI.h"
#include "heater/ZVSDriver.h"
#include "core/DeviceState.h"


struct HeatState {
    uint8_t currentCycle = 1;
    uint8_t power;
    uint16_t targetTemp;
    uint16_t temp;
    uint16_t irTemp;
    uint16_t thermoTemp;
    bool isHeating = false;
    uint32_t elapsedSeconds;
    float progress;
};

//void HeatUI(UI* _ui, HeatState state, ZVSDriver* zvs);


class UIComponent {
public:
    virtual void render(UI* _ui) = 0;
protected:
    DeviceState& ds() { return DeviceState::instance(); }
};


class HeatUI : public UIComponent {
public:
    static void render(UI* _ui, HeatState state, ZVSDriver* zvs);
private:
    static void Temperature(RenderSurface s, HeatState state);
    static void Cycle(RenderSurface s);

};