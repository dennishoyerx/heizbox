#ifndef SURFACE_FACTORY_H
#define SURFACE_FACTORY_H

#include <TFT_eSPI.h>
#include <vector>
#include <functional>

struct Rect { int16_t x, y, w, h; };

struct RenderSurface {
  TFT_eSprite *sprite = nullptr; // owned by Screen/Pool

  int16_t width() const { return sprite ? sprite->width() : 0; }
  int16_t height() const { return sprite ? sprite->height() : 0; }

  void clear(uint16_t color = TFT_BLACK) {
    if (sprite) sprite->fillSprite(color);
  }

  void blitToScreen(int16_t x, int16_t y) {
    if (sprite) sprite->pushSprite(x, y);
  }
};

using SurfaceCallback = std::function<void(RenderSurface&)>;

class SurfaceFactory {
public:
  SurfaceFactory(TFT_eSPI& tft) : _tft(&tft) {}
  ~SurfaceFactory();

  RenderSurface createSurface(int16_t w, int16_t h);
  void releaseSurface(RenderSurface& s);

  void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb);
  void usePSRAM(bool en) { _usePsram = en; }

private:
  TFT_eSPI* _tft = nullptr;
  bool _usePsram = false;

  struct PoolEntry { TFT_eSprite* sprite; int16_t w, h; };
  std::vector<PoolEntry> _pool;
};

#endif // SURFACE_FACTORY_H
