#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <map>
#include <variant>

#include "Text.hpp"
#include "RenderState.hpp"

using namespace ui;

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
  void text(int16_t x, int16_t y, const char* t, Text::Size ts = Text::Size::md, uint16_t color = 3) { Text::draw(sprite, t, Text::Config{x, y, ts, Text::Align::left, color}); }
  void text(int16_t x, int16_t y,  const String& t, Text::Size ts = Text::Size::md, uint16_t color = 3) { Text::draw(sprite, t.c_str(), Text::Config{x, y, ts, Text::Align::left, color}); }

  private:
};

