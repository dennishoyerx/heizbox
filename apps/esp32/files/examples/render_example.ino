#include <TFT_eSPI.h>
#include "render_surface.h"
#include "components.h"

TFT_eSPI tft = TFT_eSPI();
Screen screen(tft);

void setup() {
  tft.init();
  tft.setRotation(1);
  // optional: use PSRAM if available
  // screen.usePSRAM(true);
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  screen.withSurface(240, 32, 0, 0, [](RenderSurface& s) {
    Session sess = { "Session A" };
    drawSessionRow(s, Rect{0,0, s.width(), s.height()}, sess);
  });

  screen.withSurface(80, 32, 160, 0, [](RenderSurface& s) {
    TimerState st{ 3900, true }; // 65min
    drawHeatingTimer(s, Rect{0,0, s.width(), s.height()}, st);
  });

  delay(1000);
}