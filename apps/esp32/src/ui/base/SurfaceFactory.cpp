#include "ui/base/SurfaceFactory.h"

SurfaceFactory::~SurfaceFactory() {
  for (auto &e : _pool) {
    if (e.sprite) {
      e.sprite->deleteSprite();
      delete e.sprite;
      e.sprite = nullptr;
    }
  }
  _pool.clear();
}

RenderSurface SurfaceFactory::createSurface(int16_t w, int16_t h) {
  // Suche passenden Sprite im Pool
  for (auto it = _pool.begin(); it != _pool.end(); ++it) {
    if (it->sprite && it->w == w && it->h == h) {
      TFT_eSprite* spr = it->sprite;
      PoolEntry entry = *it;
      _pool.erase(it);
      RenderSurface s{ spr };
      return s;
    }
  }

  TFT_eSprite *spr = new TFT_eSprite(_tft);
  // Falls PSRAM verwendet werden soll, die Library-Version kann setPsram unterstützen
  #if defined(TFT_ESPI_HAS_SETPSRAM)
  if (_usePsram) spr->setPsram(true);
  #endif

  if (!spr->createSprite(w, h)) {
    // create failed
    spr->deleteSprite();
    delete spr;
    return RenderSurface{ nullptr };
  }
  return RenderSurface{ spr };
}

void SurfaceFactory::releaseSurface(RenderSurface& s) {
  if (!s.sprite) return;
  PoolEntry e;
  e.sprite = s.sprite;
  e.w = s.sprite->width();
  e.h = s.sprite->height();
  _pool.push_back(e);
  s.sprite = nullptr;
}

void SurfaceFactory::withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb) {
  RenderSurface s = createSurface(w, h);
  if (!s.sprite) return;
  s.clear();
  cb(s);
  s.blitToScreen(targetX, targetY);
  releaseSurface(s);
}
