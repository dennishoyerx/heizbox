#pragma once
#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include "ITemperatureSensor.h"
#include "Calibration.h"

class IRTempSensor: public ITemperatureSensor {
public:
    IRTempSensor(uint8_t sda_pin, uint8_t scl_pin, uint16_t readIntervalMs = 100);

    bool begin(float emissivity = 0.95);
    
    float read() override;

    bool setEmissivity(float emissivity);
    float getEmissivity();
    
    void enableAmbientCorrection(bool enable, float coefficient = 0.15);
    float getLastAmbientTemp();

    void setCalibration(IRCalibration::Config cal);
    IRCalibration* getCalibration() { return &calibration; }


private:
    Adafruit_MLX90614 mlx;

    uint8_t sdaPin;
    uint8_t sclPin;

    bool emissivityJustChanged;
    unsigned long emissivityChangeTime;
    static const unsigned long STABILIZATION_TIME_MS = 1000;
        
    bool ambientCorrectionEnabled;
    float ambientCorrectionCoeff;
    float referenceAmbient;
    float lastAmbient;
    
    IRCalibration calibration;
};
