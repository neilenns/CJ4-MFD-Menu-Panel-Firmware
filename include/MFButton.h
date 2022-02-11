#pragma once

#include <stdlib.h>
#include <Arduino.h>

extern "C"
{
  typedef void (*ButtonEvent)(byte, uint8_t, const __FlashStringHelper *);
};

enum
{
  btnOnPress,
  btnOnRelease,
};

/////////////////////////////////////////////////////////////////////
/// \class MFButton MFButton.h <MFButton.h>
class MFButton
{
public:
  MFButton(uint8_t pin = 1, const __FlashStringHelper * = nullptr);
  static void AttachHandler(ButtonEvent newHandler);
  void Update();
  void Trigger(uint8_t state);
  void TriggerOnPress();
  void TriggerOnRelease();
  const __FlashStringHelper *_name;
  uint8_t _pin;

private:
  static ButtonEvent _handler;
  bool _state;
};
