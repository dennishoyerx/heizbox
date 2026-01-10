#pragma once
#include <TempSensor.h>
#include <IRTempSensor.h>
#include <ITemperatureSensor.h>

class Sensors {
public:
    enum Type {
        K,
        IR
    };
    Sensors();
    void init();
    bool update(Type type = Type::K, bool ignoreInterval = false);

    uint16_t get(Type type = Type::K);

    ITemperatureSensor* getSensor(Type type = Type::K);

    TempSensor* getKSensor() { return &kSensor; }
    IRTempSensor* getIRSensor() { return &irSensor; }

    bool limitReached();

private:
    TempSensor kSensor;
    IRTempSensor irSensor;

    void handleInitializationError();
};
