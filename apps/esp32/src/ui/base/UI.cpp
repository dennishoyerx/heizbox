#include <ui/base/UI.h>
#include <ui/ColorPalette.h>
#include <ui/base/SurfaceFactory.h>

UI::UI(DisplayDriver* driver) : _driver(driver), _surfaceFactory(&driver->getTFT()) {}

RenderSurface UI::createSurface(int16_t w, int16_t h) {
    return _surfaceFactory.createSurface(w, h);
}

void UI::releaseSurface(RenderSurface& s) {
    _surfaceFactory.releaseSurface(s);
}

// Original - always renders
void UI::withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb) {
    _surfaceFactory.withSurface(w, h, targetX, targetY, cb);
}

// New - with state tracking
void UI::withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY,
                     const std::unordered_map<std::string, StateValue>& state,
                     SurfaceCallback cb) {
    _surfaceFactory.withSurface(w, h, targetX, targetY, state, cb);
}

void UI::clear() {
    _surfaceFactory.withSurface(280, 190, 0, 50, [](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
    });
}

void UI::forceRedraw() {
    _surfaceFactory.forceRedraw();
}

void UI::invalidateAll() {
    _surfaceFactory.invalidateAll();
}