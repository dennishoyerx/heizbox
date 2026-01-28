#include "driver/input/InputManager.h"
#include <Arduino.h>
#include "Config.h"

const InputManager::ButtonConfig InputManager::BUTTON_PINS[InputManager::NUM_BUTTONS] = {
    {7, ROTARY_ENCODER, ButtonSources::PCF},
    {HardwareConfig::JOY_UP_PIN, DOWN, ButtonSources::PCF},
    {HardwareConfig::JOY_DOWN_PIN, UP, ButtonSources::PCF},
    {HardwareConfig::JOY_LEFT_PIN, RIGHT, ButtonSources::PCF},
    {HardwareConfig::JOY_RIGHT_PIN, LEFT, ButtonSources::PCF},
    {HardwareConfig::JOY_PRESS_PIN, CENTER, ButtonSources::PCF},
    {HardwareConfig::FIRE_BUTTON_PIN, FIRE, ButtonSources::ESP32_GPIO},
};

InputManager::InputManager(): callback(nullptr) {}

void InputManager::onTurn(long value) {
    if (callback) callback({ROTARY_CW, ROTARY_ENCODER});
}

void InputManager::setup() {
    pcf8574 = new PCF8574(0x20);
    pcf8574->begin();

    rotaryEncoder = new RotaryEncoder(InputConfig::RotaryEncoder::CLK, InputConfig::RotaryEncoder::DT);
	rotaryEncoder->setEncoderType( EncoderType::HAS_PULLUP );
	rotaryEncoder->setBoundaries( 1, 2, false );
    rotaryEncoder->onTurned([ this ](long value) { 
        if (callback) callback({ value  == 1 ? ROTARY_CCW : ROTARY_CW, ROTARY_ENCODER}); 
    });
    rotaryEncoder->begin();

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        if (BUTTON_PINS[i].source == ButtonSources::PCF) pcf8574->pinMode(BUTTON_PINS[i].pin, INPUT);
        if (BUTTON_PINS[i].source == ButtonSources::ESP32_GPIO) pinMode(BUTTON_PINS[i].pin, INPUT_PULLUP);
    }

    Serial.println("🎮 InputManager initialized (PCF8574 + Rotary Encoder)");
}


void InputManager::update() {
    const uint32_t now = millis();

    // --- Normale Buttons ---
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        const auto& cfg = BUTTON_PINS[i];
        bool isLow;

        if (cfg.source == ButtonSources::PCF) {
            isLow = pcf8574->digitalRead(cfg.pin, true) == LOW;
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
            } else if (callback) {
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
    callback = [cb, this](InputEvent event) {
        cb(event);
        lastInputMs = millis();
    };
}