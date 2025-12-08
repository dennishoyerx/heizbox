#include "heater\HeaterCycle.h"
#include "core/DeviceState.h"

enum HeaterCycleMode {
    TEMP_DELTA,
    TEMP_CYCLES
};

HeaterCycle::HeaterCycle() {}

void HeaterCycle::init() {
}

void HeaterCycle::update() {
}

uint8_t HeaterCycle::current() {
    return DeviceState::instance().currentCycle.get();
}

uint8_t HeaterCycle::set(uint8_t cycle) {
    return DeviceState::instance().currentCycle.set(cycle);
}

uint8_t HeaterCycle::next() {
    return DeviceState::instance().currentCycle.update([](uint8_t val) { return val == 1 ? 2 : 1; });
}

bool HeaterCycle::is(uint8_t cycle) {
    return DeviceState::instance().currentCycle.get() == cycle;
}

HeaterCycle& HeaterCycle::instance() {
    static HeaterCycle heaterCycle;
    return heaterCycle;
}

/*
HeaterTemperature::HeaterTemperature() {}

uint8_t HeaterTemperature::current() {
    return 0;// DeviceState::instance().currentTemperature.get();
}

uint8_t HeaterTemperature::target() {
    return DeviceState::instance().targetTemperature.get();
}

uint8_t HeaterTemperature::setTarget(uint8_t cycle) {
    return DeviceState::instance().targetTemperature.set(cycle);
}*/