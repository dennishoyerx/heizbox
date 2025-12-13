#include <ui/base/UI.h>
#include <ui/ColorPalette.h>
#include <ui/base/SurfaceFactory.h>
#include <core/DeviceState.h>

UI::UI(DisplayDriver* driver) : _driver(driver), _surfaceFactory(&driver->getTFT()) {
    DeviceState::instance().darkMode.addListener([this](bool val) {
        _surfaceFactory.setDarkMode(val);
        clear();
    });
}

RenderSurface UI::createSurface(int16_t w, int16_t h) {
    return _surfaceFactory.createSurface(w, h);
}

void UI::releaseSurface(RenderSurface& s) {
    _surfaceFactory.releaseSurface(s);
}

void UI::withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb, bool clear) {
    _surfaceFactory.withSurface(w, h, targetX, targetY, cb, clear);
}

void UI::withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY,
                     const std::unordered_map<std::string, StateValue>& state,
                     SurfaceCallback cb, bool clear) {
    _surfaceFactory.withSurface(w, h, targetX, targetY, state, cb, clear);
}

void UI::clear() {
    _surfaceFactory.withSurface(280, 190, 0, 35, [this](RenderSurface& s) {
        s.sprite->fillSprite(COLOR_BG);
    });
    _surfaceFactory.invalidateAll();
    _surfaceFactory.forceRedraw();
}

void UI::forceRedraw() {
    _surfaceFactory.forceRedraw();
}

void UI::invalidateAll() {
    _surfaceFactory.invalidateAll();
}