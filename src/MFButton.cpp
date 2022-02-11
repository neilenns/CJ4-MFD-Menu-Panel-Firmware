#include "MFButton.h"

ButtonEvent MFButton::_handler = NULL;

MFButton::MFButton(uint8_t pin, const __FlashStringHelper *name)
{
  _pin = pin;
  _name = name;
  _state = 1;
  pinMode(_pin, INPUT_PULLUP); // set pin to input
}

void MFButton::Update()
{
  uint8_t newState = (uint8_t)digitalRead(_pin);
  if (newState != _state)
  {
    _state = newState;
    Trigger(_state);
  }
}

void MFButton::Trigger(uint8_t state)
{
  (state == LOW) ? TriggerOnPress() : TriggerOnRelease();
}

void MFButton::TriggerOnPress()
{
  if (_handler && _state == LOW)
  {
    (*_handler)(btnOnPress, _pin, _name);
  }
}

void MFButton::TriggerOnRelease()
{
  if (_handler && _state == HIGH)
  {
    (*_handler)(btnOnRelease, _pin, _name);
  }
}

void MFButton::AttachHandler(ButtonEvent newHandler)
{
  _handler = newHandler;
}