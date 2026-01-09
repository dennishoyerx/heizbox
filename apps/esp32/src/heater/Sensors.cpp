#include "heater\Sensors.h"
#include "heater\HeaterState.h"
#include "app\HeatData.h"
#include "core/DeviceState.h"
#include "Config.h"

Sensors::Sensors(): 
kSensor(HardwareConfig::THERMO_SCK_PIN, HardwareConfig::THERMO_CS_PIN, HardwareConfig::THERMO_SO_PIN, HeaterConfig::KSensor::OFF_TIME_MS),
irSensor(HardwareConfig::IR_SDA_PIN, HardwareConfig::IR_SDL_PIN) {}

void Sensors::init() {
    if (!kSensor.begin()) Serial.println("⚠️ K-Type temperature sensor initialization failed");
    if (!irSensor.begin(HeaterState::instance().irEmissivity / 100.0f)) Serial.println("⚠️ IR Temperature sensor initialization failed");
}

bool Sensors::update(Sensor type, bool ignoreInterval) {
    if (type == Sensor::IR) return irSensor.update();
    return kSensor.update(ignoreInterval);
}

uint16_t Sensors::get(Sensor type) {
    uint16_t temp = getSensor(type)->getCelsius();
    HeatLog::instance().log(type, temp);
    return temp;
}


bool Sensors::limitReached() {
    return false;
}

ITemperatureSensor* Sensors::getSensor(Sensor type) {
    if (type == Sensor::K) return &kSensor;
    if (type == Sensor::IR) return &irSensor;
    return nullptr;
}