// include/StatsScreen.h
#ifndef STATSSCREEN_H
#define STATSSCREEN_H

#include "ui/base/Screen.h"
#include "StatsManager.h"

class StatsScreen : public Screen {
public:
    StatsScreen(StatsManager& sm);

    void draw(DisplayDriver& display) override;
    void update() override;
    void handleInput(InputEvent event) override;
    ScreenType getType() const override;

private:
    StatsManager& stats;
};

#endif
