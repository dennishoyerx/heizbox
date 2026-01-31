#pragma once

#include <TFT_eSPI.h>
#include <string>
#include <string_view>

namespace ui {

class Base {
public:
  struct Config {
    int16_t x, y = 0;
  };

};
  
class Text: public Base {
public:

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
  
  struct Config: Base::Config {
    Size size = Size::md;
    Align align = Align::left;
    uint16_t color = 3;
  };
  
  
  static void draw(TFT_eSprite* sprite, std::string_view str, Config config);

private:
  static const GFXfont* getFontForSize(Size ts);

};
};
