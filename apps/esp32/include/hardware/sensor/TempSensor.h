#ifndef TEMPSENSOR_H
#define TEMPSENSOR_H

#include <Arduino.h>
#include <max6675.h>

/**
 * @brief Verwaltet die Temperaturmessung mit MAX6675 Thermoelement
 * 
 * Diese Klasse kapselt die Kommunikation mit dem MAX6675 K-Type Thermoelement
 * Verstärker und bietet Methoden zur Temperaturmessung mit Fehlerbehandlung.
 */
class TempSensor {
public:
    /**
     * @brief Konstruktor
     * @param sck_pin SCK (Clock) Pin des MAX6675
     * @param cs_pin CS (Chip Select) Pin des MAX6675
     * @param so_pin SO (MISO/Data Out) Pin des MAX6675
     */
    TempSensor(uint8_t sck_pin, uint8_t cs_pin, uint8_t so_pin);
    
    /**
     * @brief Initialisiert den Sensor
     * @return true bei erfolgreicher Initialisierung
     */
    bool begin();
    
    /**
     * @brief Liest die aktuelle Temperatur in Celsius
     * @return Temperatur in °C, oder NAN bei Fehler
     */
    float readTemperature();
    
    /**
     * @brief Liest die Temperatur mit Durchschnittsbildung
     * @param samples Anzahl der Messungen für den Durchschnitt (default: 3)
     * @param delay_ms Verzögerung zwischen Messungen in ms (default: 100)
     * @return Durchschnittstemperatur in °C, oder NAN bei Fehler
     */
    float readTemperatureAvg(uint8_t samples = 3, uint16_t delay_ms = 100);
    
    /**
     * @brief Prüft ob der Sensor einen Fehler meldet
     * @return true wenn Thermoelement nicht verbunden oder defekt
     */
    bool hasError();
    
    /**
     * @brief Gibt den letzten gültigen Temperaturwert zurück
     * @return Letzte gültige Temperatur in °C
     */
    float getLastValidTemperature() const { return lastValidTemp; }
    
    /**
     * @brief Prüft ob die Temperatur innerhalb eines sicheren Bereichs liegt
     * @param temp Zu prüfende Temperatur
     * @return true wenn im sicheren Bereich (0-500°C)
     */
    bool isTemperatureValid(float temp);
    
    /**
     * @brief Setzt Schwellwerte für Dynavap "Click"
     * @param clickTemp Temperatur bei der der Dynavap klickt (~185-195°C)
     */
    void setClickThreshold(float clickTemp = 190.0f);
    
    /**
     * @brief Prüft ob Click-Temperatur erreicht wurde
     * @return true wenn Click-Schwelle überschritten
     */
    bool hasReachedClickTemp();

private:
    MAX6675* thermocouple;
    uint8_t sckPin;
    uint8_t csPin;
    uint8_t soPin;
    
    float lastValidTemp;
    float clickThreshold;
    bool clickReached;
    
    // Fehlerzähler für Stabilität
    uint8_t errorCount;
    static const uint8_t MAX_ERRORS = 3;
    
    // Temperatur-Grenzwerte
    static constexpr float MIN_TEMP = 0.0f;
    static constexpr float MAX_TEMP = 500.0f;
    
    /**
     * @brief Validiert einen Temperaturwert
     * @param temp Zu prüfende Temperatur
     * @return true wenn Wert plausibel ist
     */
    bool validateReading(float temp);
};

#endif // TEMPSENSOR_Hv