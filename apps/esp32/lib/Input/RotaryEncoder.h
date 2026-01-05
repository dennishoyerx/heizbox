#pragma once
#include <Arduino.h>

class RotaryEncoder {
public:
  RotaryEncoder(uint8_t pinCLK, uint8_t pinDT, uint8_t pinSW = 255);

  void begin();
  void update();                 // im loop() aufrufen
  int  getPosition();            // aktuelle Position
  bool isPressed();              // Button-Event (edge)
  void reset(int value = 0);

private:
  static void IRAM_ATTR isr();
  void handleRotation();

  static RotaryEncoder* instance;

  uint8_t _pinCLK, _pinDT, _pinSW;
  volatile int  _position = 0;
  volatile bool _rotated  = false;

  bool _lastButtonState = HIGH;
};
