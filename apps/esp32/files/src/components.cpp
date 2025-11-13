#include "components.h"
#include <TFT_eSPI.h>
#include <cstdio>

void drawSessionRow(RenderSurface& target, const Rect& rect, const Session& session) {
  if (!target.sprite) return;
  TFT_eSprite *spr = target.sprite;

  spr->fillSprite(TFT_DARKGREY);
  spr->setTextColor(TFT_WHITE, TFT_DARKGREY);
  spr->setTextSize(1);
  spr->setTextDatum(TL_DATUM);
  spr->drawString(session.title ? session.title : "", 6, 6, 2);
}

void drawHeatingTimer(RenderSurface& target, const Rect& rect, const TimerState& st) {
  if (!target.sprite) return;
  TFT_eSprite *spr = target.sprite;

  spr->fillSprite(TFT_BLACK);
  spr->setTextColor(TFT_WHITE, TFT_BLACK);
  char buf[16];
  int minutes = st.seconds / 60;
  int seconds = st.seconds % 60;
  snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);
  spr->drawString(buf, 6, 8, 4);

  if (st.running) {
    // kleines laufendes Indikator-Icon oben rechts
    spr->fillCircle(rect.w - 10, 10, 4, TFT_RED);
  }
}