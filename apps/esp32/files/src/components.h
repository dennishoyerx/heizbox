#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "render_surface.h"

struct Session { const char* title; };
struct TimerState { int seconds; bool running; };

void drawSessionRow(RenderSurface& target, const Rect& rect, const Session& session);
void drawHeatingTimer(RenderSurface& target, const Rect& rect, const TimerState& st);

#endif // COMPONENTS_H