#include "hardware/sensor/TempSensor.h"

TempSensor::TempSensor(uint8_t sck_pin, uint8_t cs_pin, uint8_t so_pin)
    : sckPin(sck_pin)
    , csPin(cs_pin)
    , soPin(so_pin)
    , thermocouple(nullptr)
    , lastValidTemp(0.0f)
    , clickThreshold(190.0f)
    , clickReached(false)
    , errorCount(0)
{
}

bool TempSensor::begin() {
    // MAX6675 Objekt erstellen
    thermocouple = new MAX6675(sckPin, csPin, soPin);
    
    if (!thermocouple) {
        Serial.println("[TempSensor] ERROR: Konnte MAX6675 nicht initialisieren");
        return false;
    }
    
    // Kurze Wartezeit für Sensor-Stabilisierung
    delay(500);
    
    // Ersten Test-Reading durchführen
    float testTemp = thermocouple->readCelsius();
    
    if (isnan(testTemp)) {
        Serial.println("[TempSensor] WARNING: Erste Messung fehlgeschlagen");
        return false;
    }
    
    lastValidTemp = testTemp;
    Serial.printf("[TempSensor] Initialisiert. Start-Temperatur: %.2f°C\n", testTemp);
    
    return true;
}

float TempSensor::readTemperature() {
    if (!thermocouple) {
        Serial.println("[TempSensor] ERROR: Sensor nicht initialisiert");
        return NAN;
    }
    
    float temp = thermocouple->readCelsius();
    
    // Validierung
    if (!validateReading(temp)) {
        errorCount++;
        
        if (errorCount >= MAX_ERRORS) {
            Serial.printf("[TempSensor] ERROR: Zu viele Fehler (%d)\n", errorCount);
            return NAN;
        }
        
        // Bei einzelnen Fehlern letzten gültigen Wert zurückgeben
        Serial.println("[TempSensor] WARNING: Ungültige Messung, verwende letzten Wert");
        return lastValidTemp;
    }
    
    // Gültige Messung
    errorCount = 0;
    lastValidTemp = temp;
    
    // Click-Detection
    if (!clickReached && temp >= clickThreshold) {
        clickReached = true;
        Serial.printf("[TempSensor] Click-Temperatur erreicht! %.2f°C\n", temp);
    }
    
    return temp;
}

float TempSensor::readTemperatureAvg(uint8_t samples, uint16_t delay_ms) {
    if (samples == 0) samples = 1;
    
    float sum = 0.0f;
    uint8_t validSamples = 0;
    
    for (uint8_t i = 0; i < samples; i++) {
        float temp = readTemperature();
        
        if (!isnan(temp)) {
            sum += temp;
            validSamples++;
        }
        
        if (i < samples - 1) {
            delay(delay_ms);
        }
    }
    
    if (validSamples == 0) {
        Serial.println("[TempSensor] ERROR: Keine gültigen Messungen");
        return NAN;
    }
    
    float avg = sum / validSamples;
    Serial.printf("[TempSensor] Durchschnitt aus %d Messungen: %.2f°C\n", 
                  validSamples, avg);
    
    return avg;
}

bool TempSensor::hasError() {
    if (!thermocouple) return true;
    
    // Der MAX6675 gibt NAN zurück wenn das Thermoelement nicht verbunden ist
    float temp = thermocouple->readCelsius();
    return isnan(temp) || !isTemperatureValid(temp);
}

bool TempSensor::isTemperatureValid(float temp) {
    if (isnan(temp)) return false;
    if (temp < MIN_TEMP || temp > MAX_TEMP) return false;
    return true;
}

bool TempSensor::validateReading(float temp) {
    // Basis-Validierung
    if (!isTemperatureValid(temp)) {
        return false;
    }
    
    // Plausibilitätsprüfung: Temperatur sollte sich nicht zu schnell ändern
    // Max 50°C Sprung zwischen Messungen (bei ~4 Messungen/Sekunde des MAX6675)
    if (lastValidTemp > 0.0f) {
        float delta = abs(temp - lastValidTemp);
        if (delta > 50.0f) {
            Serial.printf("[TempSensor] WARNING: Unplausible Änderung: %.2f°C\n", delta);
            return false;
        }
    }
    
    return true;
}

void TempSensor::setClickThreshold(float clickTemp) {
    clickThreshold = clickTemp;
    clickReached = false;
    Serial.printf("[TempSensor] Click-Schwelle gesetzt: %.2f°C\n", clickTemp);
}

bool TempSensor::hasReachedClickTemp() {
    return clickReached;
}