#include <ui/base/UI.h>
#include <ui/ColorPalette.h>
#include <ui/base/SurfaceFactory.h> // Include the new header

UI::UI(DisplayDriver* driver) : _driver(driver), _surfaceFactory(&driver->getTFT()) {}

RenderSurface UI::createSurface(int16_t w, int16_t h) {
    return _surfaceFactory.createSurface(w, h);
}

void UI::releaseSurface(RenderSurface& s) {
    _surfaceFactory.releaseSurface(s);
}

void UI::withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb) {
    _surfaceFactory.withSurface(w, h, targetX, targetY, cb);
}

void UI::clear() {
    _surfaceFactory.withSurface(280, 190, 0, 50, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
    });
    //_driver->clear();
}