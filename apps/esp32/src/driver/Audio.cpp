#include "Audio.h"

namespace Audio {

// ===== Audio deaktiviert (Heap-Korruption Fix v1.7.3) =====

void init() {}
void setVolume(uint8_t percent) {}
void mute(bool on) {}

void beepMenu() {}
void beepStartup() {}
void beepError() {}
void beepSuccess() {}
void beepWarning() {}
void beepCycleUp() {}
void beepCycleDown() {}
void beepHeatStart() {}
void beepHeatFinish() {}
void beepAutoStop() {}
void rotaryTurn(bool up) {}

} // namespace Audio
