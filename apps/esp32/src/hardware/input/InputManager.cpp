#include "hardware/input/InputManager.h"
#include <Arduino.h>
#include "Config.h"


const InputManager::ButtonConfig InputManager::BUTTON_PINS[InputManager::NUM_BUTTONS] = {
    {HardwareConfig::JOY_UP_PIN, UP},
    {HardwareConfig::JOY_DOWN_PIN, DOWN},
    {HardwareConfig::JOY_LEFT_PIN, LEFT},
    {HardwareConfig::JOY_RIGHT_PIN, RIGHT},
    {HardwareConfig::JOY_PRESS_PIN, CENTER},
    {HardwareConfig::FIRE_BUTTON_PIN, FIRE}
};

// Initialisiere statisches Interrupt-Flag
volatile bool InputManager::pcfInterruptFlag = false;

// ISR: nur Flag setzen — kein I2C, keine heavy Arbeit
void IRAM_ATTR InputManager::pcfIsr() {
    InputManager::pcfInterruptFlag = true;
}

InputManager::InputManager(): callback(nullptr) {
}

void InputManager::setup() {
    buttonSource = new Pcf8574ButtonSource(0x20);
    //buttonSource->begin();

    // INT-Pin konfigurieren (PCF8574 INT ist open-drain → Pullup nutzen)
    //pinMode(InputConfig::PCF8574::INT_PIN, INPUT_PULLUP);
    // ISR registrieren: FALLING da INT low geht bei Änderung
    //attachInterrupt(digitalPinToInterrupt(InputConfig::PCF8574::INT_PIN), InputManager::pcfIsr, FALLING);

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        // RIGHT (oder andere über Expander) wird vom PCF gelesen, daher kein pinMode
        if (BUTTON_PINS[i].button != RIGHT) pinMode(BUTTON_PINS[i].pin, INPUT_PULLUP);
    }

    Serial.println("🎮 InputManager initialized (PCF8574 INT enabled)");
}

void InputManager::update() {
    const uint32_t now = millis();
    static uint32_t lastPCFUpdate = 0;

    // Wenn ISR ein Flag gesetzt hat → lese Expander aus (schnell und nur bei Bedarf)
    if (pcfInterruptFlag) {
        // clear flag before handling to avoid losing neue Interrupts während update läuft
        pcfInterruptFlag = false;
        //if (buttonSource) buttonSource->update();
        lastPCFUpdate = now;
    }

    // Fallback: falls kein Interrupt kommt, trotzdem periodisch prüfen (z.B. bei HW-Fehlern)
    constexpr uint32_t PCF_FALLBACK_MS = 1000;
    if (!pcfInterruptFlag && (now - lastPCFUpdate > PCF_FALLBACK_MS)) {
        lastPCFUpdate = now;
        //if (buttonSource) buttonSource->update();
    }

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        const auto& cfg = BUTTON_PINS[i];

        // Buttons, die über den PCF8574 laufen (hier: RIGHT) vom expander lesen,
        // andere vom GPIO.
        bool isLow;
        if (cfg.button == RIGHT && buttonSource) {
            isLow = false; //buttonSource->isPressed(i);
        } else {
            isLow = digitalRead(cfg.pin) == LOW;
        }

        const bool wasPressed = isPressed(i);

        // --- State Change Detection ---
        if (isLow && !wasPressed && (now - lastDebounce[i] > InputConfig::DEBOUNCE_MS)) {
            // --- PRESS ---
            setPressed(i, true);
            setHoldSent(i, false);
            pressTimes[i] = now;
            lastDebounce[i] = now;
            if (callback) callback({PRESS, cfg.button});

        } else if (!isLow && wasPressed) {
            // --- RELEASE ---
            setPressed(i, false);
            lastDebounce[i] = now;

            // RELEASE nur senden, wenn HOLD getriggert wurde
            if (isHoldSent(i) && callback) {
               callback({RELEASE, cfg.button});
            } else  if (callback) {
                callback({PRESSED, cfg.button});
            }

            // Reset Hold-Flag, damit erneutes Halten wieder funktioniert
            setHoldSent(i, false);
        } else if (isLow && wasPressed && !isHoldSent(i) && (now - pressTimes[i] > InputConfig::HOLD_THRESHOLD_MS)) {
            // --- HOLD ---
            setHoldSent(i, true);
            lastHoldStep[i] = now;
            if (callback) callback({HOLD_ONCE, cfg.button});
            if (callback) callback({HOLD, cfg.button});
        } else if (isLow && wasPressed && isHoldSent(i)) {
            // --- HOLDING ---
            if (now - lastHoldStep[i] >= InputConfig::HOLDING_INTERVAL_MS) {
                lastHoldStep[i] = now;
                if (callback) callback({HOLD, cfg.button});
            }
        }
    }
}

void InputManager::setCallback(EventCallback cb) {
    callback = cb;
}