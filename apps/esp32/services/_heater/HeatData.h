#pragma once
#include <Arduino.h>
#include <Heater/Sensors.h>



class HeatLog {
public:
    static HeatLog& instance() {
        static HeatLog instance;
        return instance;
    }

    void log(Sensors::Type sensor, uint16_t temp) {
        float time = (millis() - startTime) / 1000.0f;
        String data = String(time, 1) + ":" + String(temp) + ",";
        if (sensor == Sensors::Type::K) kData += data;
        if (sensor == Sensors::Type::IR) irData += data;
    }

    void start() {
        startTime = millis();
        kData = "";
        irData = "";
    }
    
const char* getKData() const {
    return kData.c_str();
}

const char* getIRData() const {
    return irData.c_str();
}



private:
    HeatLog() = default;

    String kData = "";
    String irData = "";

    uint32_t startTime;
};