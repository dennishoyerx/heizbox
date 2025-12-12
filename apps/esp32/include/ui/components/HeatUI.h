#pragma once

#include "ui/base/Screen.h"
#include "ui/base/UI.h"
#include "heater/ZVSDriver.h"
#include "core/DeviceState.h"

void drawStats(RenderSurface& s, int x, int y, String label, String value);

class UIComponent {
public:
    virtual void render(UI* _ui) = 0;
protected:
    DeviceState& ds() { return DeviceState::instance(); }
};


class HeatUI : public UIComponent {
public:
    static void render(UI* _ui, ZVSDriver* zvs);
private:
    static void Temperature(RenderSurface s);
    static void Cycle(RenderSurface s);
};