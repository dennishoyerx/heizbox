#pragma once

#include <TFT_eSPI.h>
#include <string>
#include <string_view>

namespace ui {

struct BaseConfig {
  int16_t x, y = 0;
};
  
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
  
  struct Config: BaseConfig {
    Size size = Size::md;
    Align align = Align::left;
    uint16_t color = 3;
  };
  
  static const GFXfont* getFontForSize(Size ts);
  
  static void draw(TFT_eSprite* sprite, std::string_view str, Config config) {
    if (!sprite) return;

    sprite->setTextColor(config.color);
    sprite->setFreeFont(getFontForSize(config.size));
    sprite->setTextSize(config.size != Size::xxl ? 1 : 2);

    sprite->drawString(str.data(), config.x, config.y);
  }
};
};
