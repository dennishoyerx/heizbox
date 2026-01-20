#include "_heater\heater.h"
#include "Config.h"

using namespace Heater;


Heater::Oven::Oven(ZVSDriver* zvs) : zvs(zvs) {}

void Heater::Oven::on() {
    zvs->setEnabled(true);
}

void Heater::Oven::off() {
    zvs->setEnabled(false);
}

Heater::Controller::Controller() {
    ZVSDriver* zvs = new ZVSDriver(
        HardwareConfig::HEATER_MOSFET_PIN,
        HardwareConfig::STATUS_LED_PIN
    );

    oven = new Oven(zvs);
}

void Heater::Controller::init() {}
void Heater::Controller::update() {}