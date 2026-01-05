#pragma once

#include "forward.h"

#include "ui/base/Screen.h"
#include "ui/base/UI.h"
#include "heater/ZVSDriver.h" // CONVERTED: Use forward.h
#include "core/DeviceState.h"
#include <Menu.h>

void drawStats(RenderSurface& s, int x, int y, String label, String value);

class UIComponent {
public:
    virtual void render(UI* _ui) = 0;
protected:
    DeviceState& ds() { return DeviceState::instance(); }
};


class HeatUI : public UIComponent {
public:
    static void render(UI* _ui, ZVSDriver* zvs, MenuManager* menu);
    static void Temperature(RenderSurface s);
private:
    static void Cycle(RenderSurface s);
};