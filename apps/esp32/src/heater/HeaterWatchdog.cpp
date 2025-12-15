#include "heater\HeaterWatchdog.h"

HeaterWatchdog::HeaterWatchdog(): heater(), tempWatchdog() {}

void HeaterWatchdog::init() {
    tempWatchdog.init();
}

void HeaterWatchdog::update() {
    tempWatchdog.update();
}

void HeaterWatchdog::checkTemperature() {
}