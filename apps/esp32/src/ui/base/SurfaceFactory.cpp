#include "ui/base/SurfaceFactory.h"
#include "ui/ColorPalette.h"

const GFXfont* getFontForSize(TextSize ts) {
    switch (ts) {
        case TextSize::sm: return &FreeSans9pt7b;
        case TextSize::md: return &FreeSans12pt7b;
        case TextSize::lg: return &FreeSans18pt7b;
        case TextSize::xl: return &FreeSans24pt7b;
        case TextSize::xxl: return &FreeSans18pt7b;

        case TextSize::bsm: return &FreeSansBold9pt7b;
        case TextSize::bmd: return &FreeSansBold12pt7b;
        case TextSize::blg: return &FreeSansBold18pt7b;
        case TextSize::bxl: return &FreeSansBold24pt7b;
        case TextSize::bxxl: return &FreeSansBold18pt7b;
        default:           return &FreeSans12pt7b;
    }
}

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

void SurfaceFactory::setDarkMode(bool dark) { _darkMode = dark; }

RenderSurface SurfaceFactory::createSurface(int16_t w, int16_t h, bool clear) {
  // Suche passenden Sprite im Pool
  for (auto it = _pool.begin(); it != _pool.end(); ++it) {
    if (it->sprite && it->w == w && it->h == h) {
      TFT_eSprite* spr = it->sprite;
      RenderStateHash hash = it->stateHash; // Preserve state
      _pool.erase(it);
      RenderSurface s{ spr, clear };
      s.stateHash = hash;
      return s;
    }
  }

  TFT_eSprite *spr = new TFT_eSprite(_tft);
  #if defined(TFT_ESPI_HAS_SETPSRAM)
  if (_usePsram) spr->setPsram(true);
  #endif

  spr->setColorDepth(4);
  if (!spr->createSprite(w, h)) {
    spr->deleteSprite();
    delete spr;
    return RenderSurface{ nullptr, clear };
  }

  spr->createPalette(_darkMode ? heizbox_palette_dark : heizbox_palette, 16);
  spr->fillSprite(COLOR_BG);

  return RenderSurface{ spr, clear };
}

void SurfaceFactory::releaseSurface(RenderSurface& s) {
  if (!s.sprite) return;
  PoolEntry e;
  e.sprite = s.sprite;
  e.w = s.sprite->width();
  e.h = s.sprite->height();
  e.stateHash = s.stateHash; // Store state for next use
  _pool.push_back(e);
  s.sprite = nullptr;
}

void SurfaceFactory::withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb, bool clear) {
  RenderSurface s = createSurface(w, h, clear);
  if (!s.sprite) return;
  if (clear) s.clear();
  cb(s);
  s.blitToScreen(targetX, targetY);
  releaseSurface(s);
}

void SurfaceFactory::withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY,
                                  const std::unordered_map<std::string, StateValue>& state,
                                  SurfaceCallback cb, bool clear) {
  RenderSurface s = createSurface(w, h, clear);
  if (!s.sprite) return;

  // Check if state changed - if not, skip rendering (unless force redraw)
  if (!_forceRedraw && !s.stateHash.hasChanged(state)) {
    releaseSurface(s);
    return; // State unchanged, no render needed
  }

  // State changed or forced redraw, render surface
  if (clear) s.clear();
  cb(s);
  s.blitToScreen(targetX, targetY);
  releaseSurface(s);
  
  // Reset force redraw flag after first use
  _forceRedraw = false;
}