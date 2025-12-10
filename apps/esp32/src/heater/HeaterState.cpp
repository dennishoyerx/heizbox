#include "heater/HeaterState.h"

HeaterState& HeaterState::instance() {
    static HeaterState state;
    return state;
}


HeaterState& hs() {
    return HeaterState::instance();
}