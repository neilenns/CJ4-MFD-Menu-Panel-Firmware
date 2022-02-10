#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <MCP23017.h>

enum DetectionState
{
  WaitingForPress,
  PressDetected,
  WaitingForRelease
};

enum ButtonState
{
  Pressed,
  Released,
};

extern "C"
{
  typedef void (*KeyboardEvent)();
  typedef void (*ButtonEvent)(ButtonState, uint8_t);
};

class ExpanderManager
{
private:
  uint8_t _activeButton = 0;
  ButtonEvent _buttonHandler;
  volatile DetectionState _currentState = DetectionState::WaitingForPress;
  KeyboardEvent _interruptHandler;
  uint8_t _interruptPin;
  unsigned long _lastPressEventTime;

  MCP23017 *_mcp;

  void CheckForButton();
  void CheckForRelease();
  static int GetBitPosition(uint16_t value);

public:
  ExpanderManager(uint8_t address, uint8_t interruptPin, KeyboardEvent interruptHandler, ButtonEvent buttonHandler);
  void Init();
  void Loop();
  void HandleInterrupt();
};