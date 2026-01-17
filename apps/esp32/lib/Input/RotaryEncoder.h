#pragma once
#include <Arduino.h>

class RotaryEncoder {
public:
  RotaryEncoder(uint8_t pinCLK, uint8_t pinDT, uint8_t pinSW = 255);
  
  void begin();
  void update();
  
  int getPosition();
  bool isPressed();
  void reset(int value = 0);

private:
  uint8_t _pinCLK;
  uint8_t _pinDT;
  uint8_t _pinSW;

  volatile int _position = 0;
  volatile bool _rotated = false;
  volatile bool _lastCLK = LOW;
  volatile bool _lastDT = LOW;

  bool _lastButtonState = HIGH;

  static RotaryEncoder* instance;
  static void IRAM_ATTR isr();
  void handleRotation();
};