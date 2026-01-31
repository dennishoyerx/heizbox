#include "Text.hpp"

using namespace ui;

void Text::draw(TFT_eSprite* sprite, std::string_view str, Config config) {
  if (!sprite) return;

  sprite->setTextColor(config.color);
  sprite->setFreeFont(getFontForSize(config.size));
  sprite->setTextSize(config.size != Size::xxl ? 1 : 2);

  sprite->drawString(str.data(), config.x, config.y);
}
