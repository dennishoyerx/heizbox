#include "TempSensor.h"
#include <MAX6675.h>
#include <Arduino.h>

TempSensor::TempSensor(uint8_t sck_pin, uint8_t cs_pin, uint8_t so_pin, uint16_t readIntervalMs)
    : sckPin(sck_pin), csPin(cs_pin), soPin(so_pin), ITemperatureSensor(readIntervalMs)
{
    thermocouple = new MAX6675(sck_pin, cs_pin, so_pin);
}

bool TempSensor::begin() {
    float temp = thermocouple->readCelsius();
    if (isnan(temp)) return false;
    lastValidTemp = temp;
    lastReadTime = millis();
    return true;
}

float TempSensor::read() {
    return thermocouple->readCelsius();
}
