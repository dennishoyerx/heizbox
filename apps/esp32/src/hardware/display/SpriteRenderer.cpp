// apps/esp32/src/hardware/display/SpriteRenderer.cpp
#include "hardware/display/SpriteRenderer.h"
#include "ui/ColorPalette.h"
#include "hardware/display/DisplayDriver.h" // Include the header that defines DisplayConfig

SpriteRenderer::SpriteRenderer(TFT_eSPI* tft)
    : tft(tft), mainSprite(tft), spriteAllocated(false) {}

SpriteRenderer::~SpriteRenderer() {
    freeSprites();
}

void SpriteRenderer::freeSprites() {
    if (spriteAllocated) {
        mainSprite.deleteSprite();
        spriteAllocated = false;
        Serial.println("🗑️ Sprite buffer freed");
    }
}

void SpriteRenderer::reallocateSprites(uint16_t w, uint16_t h, uint8_t color_depth) {
    freeSprites();

    mainSprite.setColorDepth(color_depth);
    spriteAllocated = mainSprite.createSprite(w, h);

    if (spriteAllocated) {
        //mainSprite.createPalette(heizbox_palette, 16);
        //mainSprite.fillSprite(COLOR_ACCENT);

        const size_t bytes = w * h * (color_depth / 8);
        Serial.printf("✅ Sprite allocated with custom palette: %u bytes (%ux%u @%d-bit)\n",
                      bytes, w, h, color_depth);
    } else {
        Serial.println("❌ Sprite allocation failed - fallback to direct rendering");
        Serial.printf("   Required: %u bytes\n", w * h * (color_depth / 8));
    }
}

TFT_eSprite& SpriteRenderer::getSprite() {
    return mainSprite;
}

bool SpriteRenderer::isAllocated() const {
    return spriteAllocated;
}

void SpriteRenderer::push() {
    if (spriteAllocated) {
        mainSprite.pushSprite(0, DisplayConfig::STATUS_BAR_HEIGHT);
    }
}
