#include "heater\HeaterTemperature.h"
#include "heater\HeaterState.h"
#include "heater\HeatData.h"
#include "core/DeviceState.h"
#include "Config.h"

HeaterTemperature::HeaterTemperature(): 
kSensor(HardwareConfig::THERMO_SCK_PIN, HardwareConfig::THERMO_CS_PIN, HardwareConfig::THERMO_SO_PIN, HeaterConfig::SENSOR_OFF_TIME_MS),
irSensor(HardwareConfig::IR_SDA_PIN, HardwareConfig::IR_SDL_PIN) {}

void HeaterTemperature::init() {
    if (!kSensor.begin()) Serial.println("⚠️ K-Type temperature sensor initialization failed");
    if (!irSensor.begin()) Serial.println("⚠️ IR Temperature sensor initialization failed");
}

bool HeaterTemperature::update(Sensor type, bool ignoreInterval) {
    if (type == Sensor::IR) return irSensor.update();
    return kSensor.update(ignoreInterval);
}

uint16_t HeaterTemperature::get(Sensor type) {
    uint16_t temp = getSensor(type)->getCelsius();
    HeatLog::instance().log(type, temp);
    return temp;
}


bool HeaterTemperature::limitReached() {
    return false;
}

ITemperatureSensor* HeaterTemperature::getSensor(Sensor type) {
    if (type == Sensor::K) return &kSensor;
    if (type == Sensor::IR) return &irSensor;
    return nullptr;
}