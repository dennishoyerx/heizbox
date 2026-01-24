#pragma once

#include <cstdint>

class ICalibration {};

class IRTwoPointCalibration: public ICalibration {
public:
    enum PointId {
        A,
        B
    };

    struct PointConfig {
        uint16_t measured;
        uint16_t actual;
    };

    struct BaseConfig {
        PointConfig a;
        PointConfig b;
        float slope;
        float offset;
    };

    explicit IRTwoPointCalibration();
    
    void compute();
    void clear();

    void setPointConfig(PointId p, PointConfig pc);
    PointConfig getPointConfig(PointId p);
    float getSlope();
    float getOffset();

private:
    BaseConfig config;
};
