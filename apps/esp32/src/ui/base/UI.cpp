#include <ui/base/UI.h>

UI::UI(DisplayDriver* driver) : _driver(driver) {}

TFT_eSprite* UI::createSprite(int16_t w, int16_t h) {
    return _driver->createSprite(w, h);
}
