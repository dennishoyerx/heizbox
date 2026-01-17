#include "RotaryEncoder.h"

RotaryEncoder* RotaryEncoder::instance = nullptr;

RotaryEncoder::RotaryEncoder(uint8_t pinCLK, uint8_t pinDT, uint8_t pinSW)
: _pinCLK(pinCLK), _pinDT(pinDT), _pinSW(pinSW) {}

void RotaryEncoder::begin() {
  instance = this;

  pinMode(_pinCLK, INPUT_PULLUP);
  pinMode(_pinDT,  INPUT_PULLUP);

  if (_pinSW != 255) {
    pinMode(_pinSW, INPUT_PULLUP);
  }

  // Initialen Zustand lesen
  _lastCLK = digitalRead(_pinCLK);
  _lastDT = digitalRead(_pinDT);

  attachInterrupt(digitalPinToInterrupt(_pinCLK), isr, CHANGE);
}

void IRAM_ATTR RotaryEncoder::isr() {
  if (instance) {
    instance->handleRotation();
  }
}

void RotaryEncoder::handleRotation() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = micros();
  
  // Debounce: Ignoriere Interrupts die schneller als 1ms aufeinander folgen
  if (interruptTime - lastInterruptTime < 1000) {
    return;
  }
  
  bool currentCLK = digitalRead(_pinCLK);
  bool currentDT = digitalRead(_pinDT);
  
  // Nur auf Zustandswechsel von CLK reagieren
  if (currentCLK != _lastCLK) {
    // Wenn CLK und DT unterschiedlich sind -> CW
    // Wenn CLK und DT gleich sind -> CCW
    if (currentCLK != currentDT) {
      _position++;
    } else {
      _position--;
    }
    _rotated = true;
  }
  
  _lastCLK = currentCLK;
  _lastDT = currentDT;
  lastInterruptTime = interruptTime;
}

void RotaryEncoder::update() {
  // aktuell leer, vorbereitet für Acceleration / Filtering
}

int RotaryEncoder::getPosition() {
  noInterrupts();
  int pos = _position;
  interrupts();
  return pos;
}

bool RotaryEncoder::isPressed() {
  if (_pinSW == 255) return false;

  bool current = digitalRead(_pinSW);
  bool pressed = (_lastButtonState == HIGH && current == LOW);
  _lastButtonState = current;

  if (pressed) delay(30); // simples Debounce
  return pressed;
}

void RotaryEncoder::reset(int value) {
  noInterrupts();
  _position = value;
  interrupts();
}