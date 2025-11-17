#pragma once

#include <hardware/display/DisplayDriver.h>
#include <ui/base/SurfaceFactory.h>
#include <unordered_map>
#include <string>

class UI {
public:
    UI(DisplayDriver* driver);

    // Surface
    RenderSurface createSurface(int16_t w, int16_t h);
    void releaseSurface(RenderSurface& s);
    void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb);
    void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY,
                     const std::unordered_map<std::string, StateValue>& state,
                     SurfaceCallback cb);

    void clear();
    
    // Force all surfaces to redraw on next render cycle
    void forceRedraw();
    
    // Invalidate all cached surface states
    void invalidateAll();

private:
    DisplayDriver* _driver;
    SurfaceFactory _surfaceFactory;
};