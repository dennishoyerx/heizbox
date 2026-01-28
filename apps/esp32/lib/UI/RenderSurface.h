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

namespace ui {

struct Text {
  enum class Size {
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

  enum class Weight {
    light,
    normal,
    bold
  };
  
  enum class Align {
      left,
      right,
      center
  };

  struct BaseConfig {
    int16_t x, y = 0;
  };

  struct Config: BaseConfig {
    std::string_view text;
    Size size = Size::md;
    Align align = Align::left;
    uint16_t color = 3;
  };

  void draw(TFT_eSprite* sprite, Config config) {
    if (!sprite) return;

    sprite->setTextColor(config.color);
    //sprite->setFreeFont(getFontForSize(config.size));
    sprite->setTextSize(config.size != Size::xxl ? 1 : 2);

    //sprite->drawString(config.text, config.x, config.y);
  }
};
};

enum class TextSize  {
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
using namespace ui;
const GFXfont* getFontForSize(Text::Size ts);

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
  void text(int16_t x, int16_t y, const char* t, Text::Size ts = Text::Size::md, uint16_t color = 3) { drawText(x, y, String(t), ts, color); }
  void text(int16_t x, int16_t y, const String& t, Text::Size ts = Text::Size::md, uint16_t color = 3) { drawText(x, y, t, ts, color); }
  //void text(int16_t x, int16_t y, const String& t, TextConfig config = TextConfig()) { drawText(x, y, t, config.size, config.color); }

  private:
    void drawText(int16_t x, int16_t y, const String& t, Text::Size ts = Text::Size::md, uint16_t color = 3) {
      if (!sprite) return;

      sprite->setTextColor(color);
      sprite->setFreeFont(getFontForSize(ts));
      sprite->setTextSize(ts != Text::Size::xxl ? 1 : 2);

      sprite->drawString(t, x, y);
  }
};

