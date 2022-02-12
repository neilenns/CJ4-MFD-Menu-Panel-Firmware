#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <MCP23017.h>

enum DetectionState
{
  WaitingForPress,
  WaitingForRelease
};

enum ButtonState
{
  Pressed,
  Released,
};

extern "C"
{
  typedef void (*ExpanderEvent)(ButtonState, uint8_t, uint8_t);
};

class ExpanderManager
{
private:
  uint8_t _activeButton = 0;
  ExpanderEvent _buttonHandler;
  volatile DetectionState _currentState = DetectionState::WaitingForPress;
  uint8_t _deviceAddress;

  MCP23017 *_mcp;

  void CheckForButton();
  void CheckForRelease();
  static int GetBitPosition(uint16_t value);

public:
  ExpanderManager(uint8_t address, ExpanderEvent buttonHandler);
  void Init();
  void Loop();
};