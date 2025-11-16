#pragma once

#include <hardware/display/DisplayDriver.h>
#include <ui/base/SurfaceFactory.h>

class HeaterLogic {
public:
    HeaterLogic(DisplayDriver* driver);
    RenderSurface createSurface(int16_t w, int16_t h);
    void releaseSurface(RenderSurface& s);
    void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb);

    void clear();

private:
    DisplayDriver* _driver;
    SurfaceFactory _surfaceFactory;
};
