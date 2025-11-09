// apps/esp32/include/hardware/display/SpriteRenderer.h
#ifndef SPRITERENDERER_H
#define SPRITERENDERER_H

#include <TFT_eSPI.h>

class SpriteRenderer {
public:
    explicit SpriteRenderer(TFT_eSPI* tft);
    ~SpriteRenderer();

    void reallocateSprites(uint16_t w, uint16_t h, uint8_t color_depth);
    void freeSprites();
    
    TFT_eSprite& getSprite();
    bool isAllocated() const;

    void push();

private:
    TFT_eSPI* tft;
    TFT_eSprite mainSprite;
    bool spriteAllocated;
};

#endif // SPRITERENDERER_H
