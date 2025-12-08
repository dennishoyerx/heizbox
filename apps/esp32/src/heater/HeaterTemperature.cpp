#include "heater\HeaterTemperature.h"
#include "Config.h"

HeaterTemperature::HeaterTemperature(): 
kSensor(HardwareConfig::THERMO_SCK_PIN, HardwareConfig::THERMO_CS_PIN, HardwareConfig::THERMO_SO_PIN, HeaterConfig::SENSOR_OFF_TIME_MS),
irSensor(HardwareConfig::IR_SDA_PIN, HardwareConfig::IR_SDL_PIN) {}

void HeaterTemperature::init() {
    if (!kSensor.begin()) Serial.println("⚠️ K-Type temperature sensor initialization failed");
    if (!irSensor.begin()) Serial.println("⚠️ IR Temperature sensor initialization failed");
}

void HeaterTemperature::update(HeaterTemperatures type, bool ignoreInterval) {
    if (type == K || type == MAIN) kSensor.update(ignoreInterval);
    if (type == IR) irSensor.update();
}

uint16_t HeaterTemperature::get(HeaterTemperatures type) {
    uint16_t temp;
    if (type == K || type == MAIN) temp = kSensor.getTemperature();
    if (type == IR) temp = irSensor.getTemperature();

    return validate(temp);
}

uint16_t HeaterTemperature::validate(uint16_t temp) {
    return temp < 300 ? temp : 3;
}