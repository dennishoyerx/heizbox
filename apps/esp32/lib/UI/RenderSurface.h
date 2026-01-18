#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <map>
#include <variant>

struct Rect { int16_t x, y, w, h; };

enum class TextSize {
    xs,
    sm,
    md,
    lg,
    xl,
    xxl,

    bsm,
    bmd,
    blg,
    bxl,
    bxxl
};

enum class TextAlign {
    left,
    right,
    center
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
  bool clean = true;
  RenderStateHash stateHash;
  std::map<std::string, StateValue> state;

  RenderSurface(TFT_eSprite* s = nullptr, bool clear = true) : sprite(s), clean(clear) {}

  int16_t width() const { return sprite ? sprite->width() : 0; }
  int16_t height() const { return sprite ? sprite->height() : 0; }
  int16_t centerX() const { return width() / 2; }
  int16_t centerY() const { return height() / 2; }
  int16_t top() const { return 0; }
  int16_t bottom() const { return height(); }
  int16_t left() const { return 0; }
  int16_t right() const { return width(); }

  void clear(uint16_t color = TFT_BLACK) { if (sprite && clean) sprite->fillSprite(color); }
  void blitToScreen(int16_t x, int16_t y) { if (sprite) sprite->pushSprite(x, y); }
  void text(int16_t x, int16_t y, const char* t, TextSize ts = TextSize::md, uint16_t color = 3) { _text(x, y, String(t), ts, color); }
  void text(int16_t x, int16_t y, const String& t, TextSize ts = TextSize::md, uint16_t color = 3) { _text(x, y, t, ts, color); }

  private:
    void _text(int16_t x, int16_t y, const String& t, TextSize ts = TextSize::md, uint16_t color = 3) {
      if (!sprite) return;

      sprite->setTextColor(color);
      sprite->setFreeFont(getFontForSize(ts));
      sprite->setTextSize(ts != TextSize::xxl ? 1 : 2);

      sprite->drawString(t, x, y);
  }
};