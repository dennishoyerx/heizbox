#include "heater\HeaterCycle.h"
#include "heater\HeaterState.h"

HeaterCycle::HeaterCycle() {}

void HeaterCycle::init() {
}

void HeaterCycle::update() {
}

uint8_t HeaterCycle::current() {
    return HeaterState::instance().cycle;
}

uint8_t HeaterCycle::set(uint8_t cycle) {
    return HeaterState::instance().cycle.set(cycle);
}

uint8_t HeaterCycle::next() {
    return HeaterState::instance().cycle.update([](uint8_t val) { return val == 1 ? 2 : 1; });
}

bool HeaterCycle::is(uint8_t cycle) {
    return HeaterState::instance().cycle == cycle;
}

HeaterCycle& HeaterCycle::instance() {
    static HeaterCycle heaterCycle;
    return heaterCycle;
}