#ifndef STATSSCREEN_H
#define STATSSCREEN_H

#include "Screen.h"
#include "DisplayManager.h"
#include "StatsManager.h"
#include "ScreenType.h"
#include "InputManager.h"

class StatsScreen : public Screen {
private:
    StatsManager& stats;

public:
    StatsScreen(StatsManager& sm);
    void draw(DisplayManager& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;
};

#endif