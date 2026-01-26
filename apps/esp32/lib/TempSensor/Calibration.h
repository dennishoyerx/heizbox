#pragma once

#include <cstdint>

class ICalibration {
public:
    bool hasConfig();
};

class IRCalibration: public ICalibration {
public:
    struct PointConfig {
        uint16_t measured;
        uint16_t actual;
    };

    struct Config {
        PointConfig a;
        PointConfig b;
        float slope;
        float offset;
    };

    explicit IRCalibration();
    
    void compute();
    void clear();

    void setConfig(Config const c);
    bool const hasConfig();
    Config const getConfig() { return config; }

    float const getSlope() { return config.slope; }
    float const getOffset() { return config.offset; }

    uint16_t const processTemperature(uint16_t temp);

private:
    Config config;
};
