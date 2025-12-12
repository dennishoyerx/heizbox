#pragma once
#include <Arduino.h>

String formatConsumption(float consumption) {
        char consumptionStr[10];
        int integer = (int)consumption;
        int decimal = ((int)(consumption * 100 + 0.5f)) % 100;
        if (integer > 0) {
            sprintf(consumptionStr, "%d.%02dg", integer, decimal);
        }
        else {
            sprintf(consumptionStr, ".%02dg", decimal);
        }
    return (String) consumptionStr;
}