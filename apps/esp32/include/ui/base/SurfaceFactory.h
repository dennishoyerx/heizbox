#pragma once

#include "ui/ColorPalette.h"
#include <TFT_eSPI.h>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <variant>

struct Rect { int16_t x, y, w, h; };

enum class TextSize {
    sm,
    md,
    lg,
    xl
};

// Variant type for different state value types
using StateValue = std::variant<int, float, bool, std::string>;

// State hash for change detection
struct RenderStateHash {
  std::unordered_map<std::string, StateValue> values;
  size_t hash = 0;

  // Calculate hash from all values
  size_t calculateHash() const {
    size_t h = 0;
    for (const auto& [key, value] : values) {
      h ^= std::hash<std::string>{}(key);
      std::visit([&h](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
          h ^= std::hash<std::string>{}(arg);
        } else if constexpr (std::is_same_v<T, float>) {
          h ^= std::hash<int>{}(static_cast<int>(arg * 1000)); // float precision
        } else {
          h ^= std::hash<T>{}(arg);
        }
      }, value);
    }
    return h;
  }

  bool hasChanged(const std::unordered_map<std::string, StateValue>& newValues) {
    size_t newHash = 0;
    for (const auto& [key, value] : newValues) {
      newHash ^= std::hash<std::string>{}(key);
      std::visit([&newHash](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
          newHash ^= std::hash<std::string>{}(arg);
        } else if constexpr (std::is_same_v<T, float>) {
          newHash ^= std::hash<int>{}(static_cast<int>(arg * 1000));
        } else {
          newHash ^= std::hash<T>{}(arg);
        }
      }, value);
    }

    if (newHash != hash) {
      hash = newHash;
      values = newValues;
      return true;
    }
    return false;
  }
};

const GFXfont* getFontForSize(TextSize ts);

struct RenderSurface {
  TFT_eSprite *sprite = nullptr;
  RenderStateHash stateHash;

  RenderSurface(TFT_eSprite* s = nullptr) : sprite(s) {}

  int16_t width() const { return sprite ? sprite->width() : 0; }
  int16_t height() const { return sprite ? sprite->height() : 0; }

  void clear(uint16_t color = TFT_BLACK) {
    if (sprite) sprite->fillSprite(color);
  }

  void blitToScreen(int16_t x, int16_t y) {
    if (sprite) sprite->pushSprite(x, y);
  }

  void text(int16_t x, int16_t y, const char* t, TextSize ts = TextSize::md, uint16_t color = COLOR_TEXT_PRIMARY) {
    _text(x, y, String(t), ts, color);
  }

  void text(int16_t x, int16_t y, const String& t, TextSize ts = TextSize::md, uint16_t color = COLOR_TEXT_PRIMARY) {
      _text(x, y, t, ts, color);
  }

  private:
    void _text(int16_t x, int16_t y, const String& t, TextSize ts = TextSize::md, uint16_t color = COLOR_TEXT_PRIMARY) {
      if (!sprite) return;
      sprite->setTextColor(color);
      sprite->setFreeFont(getFontForSize(ts));
      sprite->drawString(t, x, y);
  }
};

using SurfaceCallback = std::function<void(RenderSurface&)>;

class SurfaceFactory {
public:
  SurfaceFactory(TFT_eSPI* tft) : _tft(tft), _usePsram(false), _forceRedraw(false) {}
  ~SurfaceFactory();

  RenderSurface createSurface(int16_t w, int16_t h);
  void releaseSurface(RenderSurface& s);

  // Original withSurface - always renders
  void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb);

  // New withSurface with state tracking - only renders if state changed
  void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, 
                   const std::unordered_map<std::string, StateValue>& state,
                   SurfaceCallback cb);

  void usePSRAM(bool en) { _usePsram = en; }
  
  // Force all surfaces to redraw on next render
  void forceRedraw() { _forceRedraw = true; }
  
  // Invalidate all cached states in the pool
  void invalidateAll() {
    for (auto& entry : _pool) {
      entry.stateHash.hash = 0;
      entry.stateHash.values.clear();
    }
  }

private:
  TFT_eSPI* _tft;
  bool _usePsram;
  bool _forceRedraw;

  struct PoolEntry { 
    TFT_eSprite* sprite = nullptr; 
    int16_t w = 0;
    int16_t h = 0;
    RenderStateHash stateHash;
  };
  std::vector<PoolEntry> _pool;
};
