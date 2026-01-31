#pragma once

#include <TFT_eSPI.h>
#include <string>
#include <string_view>
#include "driver/input/InputManager.h"
#include <functional>

namespace ui {

class ScreenInput {

};

};

struct ScreenInput {
  InputButton button;
  std::function<void()> callback;
};

struct x {
  ScreenInput x{InputButton::CENTER, [](){}};
};



void xx() {

};