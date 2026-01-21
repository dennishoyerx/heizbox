#include "heater\Sensors.h"
#include "heater\HeaterState.h"
#include "core/DeviceState.h"
#include "Config.h"

Sensors::Sensors(): 
kSensor(HardwareConfig::THERMO_SCK_PIN, HardwareConfig::THERMO_CS_PIN, HardwareConfig::THERMO_SO_PIN, HeaterConfig::KSensor::OFF_TIME_MS),
irSensor(HardwareConfig::SDA_PIN, HardwareConfig::SCL_PIN) {}

void Sensors::init() {
    //if (!kSensor.begin()) Serial.println("⚠️ K-Type temperature sensor initialization failed");
    if (!irSensor.begin(HeaterState::instance().irEmissivity / 100.0f)) Serial.println("⚠️ IR Temperature sensor initialization failed");
}

bool Sensors::update(Type type, bool ignoreInterval) {
    if (type == Type::IR) return irSensor.update();
    return kSensor.update(ignoreInterval);
}

uint16_t Sensors::get(Type type) {
    uint16_t temp = getSensor(type)->getCelsius();
    return temp;
}

ITemperatureSensor* Sensors::getSensor(Type type) {
    if (type == Type::K) return &kSensor;
    if (type == Type::IR) return &irSensor;
    return nullptr;
}