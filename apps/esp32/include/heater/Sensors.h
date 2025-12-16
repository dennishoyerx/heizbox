#pragma once
#include <TempSensor.h>
#include <IRTempSensor.h>
#include <ITemperatureSensor.h>

class Sensors {
public:
    enum Sensor {
        K,
        IR
    };
    Sensors();
    void init();
    bool update(Sensor type = Sensor::K, bool ignoreInterval = false);

    uint16_t get(Sensor type = Sensor::K);

    ITemperatureSensor* getSensor(Sensor type = Sensor::K);

    TempSensor* getKSensor() { return &kSensor; }
    IRTempSensor* getIRSensor() { return &irSensor; }

    bool limitReached();

private:
    TempSensor kSensor;
    IRTempSensor irSensor;

    void handleInitializationError();
};
