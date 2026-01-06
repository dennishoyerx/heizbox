#include "IRTempSensor.h" 
 
IRTempSensor::IRTempSensor(uint8_t sda_pin, uint8_t scl_pin, uint16_t readIntervalMs) 
    : sdaPin(sda_pin), 
      sclPin(scl_pin), 
      ITemperatureSensor(readIntervalMs),
      emissivityJustChanged(false),
      emissivityChangeTime(0),
      ambientCorrectionEnabled(false),
      ambientCorrectionCoeff(0.15),
      referenceAmbient(25.0),
      lastAmbient(25.0) { 
    pinMode(sda_pin, INPUT_PULLUP); 
    pinMode(scl_pin, INPUT_PULLUP); 
} 
 
bool IRTempSensor::begin(float emissivity) { 
    Wire.begin(sdaPin, sclPin); 
 
    if (!mlx.begin()) { 
        errorCount++; 
        return false; 
    } 
 
    // Sensor initial stabilisieren
    delay(250);
    
    // Referenz-Ambient-Temperatur erfassen
    float ambientSum = 0;
    for(int i = 0; i < 5; i++) {
        ambientSum += mlx.readAmbientTempC();
        delay(20);
    }
    referenceAmbient = ambientSum / 5.0;
    lastAmbient = referenceAmbient;
    
    // Emissivity nur setzen wenn nötig
    float currentEmissivity = mlx.readEmissivity();
    if (abs(currentEmissivity - emissivity) > 0.01) {
        setEmissivity(emissivity);
    }
 
    lastValidTemp = NAN; 
    errorCount = 0; 
    return true; 
} 
 
float IRTempSensor::read() {
    // Nach Emissivitätsänderung: Stabilisierungszeit abwarten
    if (emissivityJustChanged) {
        if (millis() - emissivityChangeTime < STABILIZATION_TIME_MS) {
            // Während Stabilisierung: Dummy-Reads ohne Rückgabe
            mlx.readObjectTempC();
            mlx.readAmbientTempC();
            return lastValidTemp; // Alten Wert zurückgeben
        } else {
            emissivityJustChanged = false;
        }
    }
    
    // Beide Temperaturen lesen
    float objTemp = mlx.readObjectTempC();
    float ambTemp = mlx.readAmbientTempC();
    lastAmbient = ambTemp;
    
    // Optional: Ambient-Korrektur anwenden
    if (ambientCorrectionEnabled) {
        float ambientDelta = ambTemp - referenceAmbient;
        objTemp -= ambientDelta * ambientCorrectionCoeff;
    }
    
    return objTemp; 
} 
 
bool IRTempSensor::setEmissivity(float emissivity) { 
    if (emissivity < 0.1 || emissivity > 1.0) return false; 
    
    // Prüfen ob Änderung überhaupt nötig ist
    float current = mlx.readEmissivity();
    if (abs(current - emissivity) < 0.01) {
        return true; // Bereits richtig eingestellt
    }
    
    // EEPROM-Write durchführen
    mlx.writeEmissivity(emissivity);   
    delay(10); // EEPROM Write Zeit abwarten
    
    // Sensor-Reset triggern (falls die Library das unterstützt)
    // Alternativ: I2C-Bus kurz resetten
    Wire.end();
    delay(50);
    Wire.begin(sdaPin, sclPin);
    delay(50);
    
    if (!mlx.begin()) {
        return false;
    }
    
    delay(100);
    
    // Ausgiebig stabilisieren: Mehrere Dummy-Reads
    for(int i = 0; i < 10; i++) { 
        mlx.readObjectTempC();
        mlx.readAmbientTempC(); 
        delay(50); 
    }
    
    // Stabilisierungs-Flag setzen
    emissivityJustChanged = true;
    emissivityChangeTime = millis();
    
    // Verifizieren
    float verify = mlx.readEmissivity();
    return abs(verify - emissivity) < 0.01; 
} 
 
float IRTempSensor::getEmissivity() { 
    return mlx.readEmissivity(); 
}

void IRTempSensor::enableAmbientCorrection(bool enable, float coefficient) {
    ambientCorrectionEnabled = enable;
    if (coefficient >= 0.0 && coefficient <= 1.0) {
        ambientCorrectionCoeff = coefficient;
    }
}

float IRTempSensor::getLastAmbientTemp() {
    return lastAmbient;
}