#include "heater/Calibration.h"

IRCalibration::IRCalibration() {}

void IRCalibration::compute() {}
void IRCalibration::clear() {}

 void xx() {
    IRCalibration c = IRCalibration();
    IRCalibration::PointConfig pc = IRCalibration::PointConfig();
    pc.actual = 1;
    pc.measured = 2;

    c.setPointConfig(IRCalibration::PointId::A, { 
        .measured = 2,
        .actual = 1,
    });
 }

/*
int16_t HeaterController::markIRClick(uint16_t actualTemp) {
    auto& hs = HeaterState::instance();

    temperature.update(Sensors::Type::IR, true);
    float raw = temperature.get(Sensors::Type::IR);

    if (!isfinite(raw) || raw <= 0.0f || raw > 1000.0f) {
        Serial.println("IR click: invalid measurement, ignored.");
        return -1;
    }
    uint16_t measured = static_cast<uint16_t>(raw + 0.5f);
    int16_t returnVal = -1;

    // If actualTemp matches one of the stored actuals, use that slot. Otherwise pick an empty slot (A first).
    if (actualTemp == hs.irCalActualA) {
        returnVal = hs.irCalMeasuredA.set(measured);
        Serial.printf("IR click stored in A: measured=%u actual=%u\n", measured, actualTemp);
    } else if (actualTemp == hs.irCalActualB) {
        returnVal = hs.irCalMeasuredB.set(measured);
        Serial.printf("IR click stored in B: measured=%u actual=%u\n", measured, actualTemp);
    } else {
        if (hs.irCalMeasuredA == 0) {
            returnVal = hs.irCalMeasuredA.set(measured);
            hs.irCalActualA.set(actualTemp);
            Serial.printf("IR click stored in A (new actual): measured=%u actual=%u\n", measured, actualTemp);
        } else {
            returnVal = hs.irCalMeasuredB.set(measured);
            hs.irCalActualB.set(actualTemp);
            Serial.printf("IR click stored in B (new actual): measured=%u actual=%u\n", measured, actualTemp);
        }
    }

    computeIRCalibration();
    return returnVal;
}

void HeaterController::computeIRCalibration() {
    auto& hs = HeaterState::instance();
    uint16_t mA = hs.irCalMeasuredA;
    uint16_t mB = hs.irCalMeasuredB;
    uint16_t aA = hs.irCalActualA;
    uint16_t aB = hs.irCalActualB;

    if (mA == 0 || mB == 0) {
        Serial.println("IR calibration: need two measured points.");
        return;
    }
    if (mA == mB) {
        Serial.println("IR calibration: measured points identical, cannot compute.");
        return;
    }

    float slope = float(aB - aA) / float(mB - mA);
    float offset = float(aA) - slope * float(mA);

    hs.irCalSlope.set(slope);
    hs.irCalOffset.set(offset);

    Serial.printf("IR calibration computed: slope=%.6f offset=%.2f (mA=%u,aA=%u mB=%u,aB=%u)\n",
                  slope, offset, mA, aA, mB, aB);
}*/