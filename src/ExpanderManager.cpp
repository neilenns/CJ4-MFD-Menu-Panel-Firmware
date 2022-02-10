#include <Arduino.h>
#include <Wire.h>
#include <MCP23017.h>

#include "ExpanderManager.h"

constexpr unsigned long DEBOUNCE_TIME_MS = 10;          // Time between button events in milliseconds.
constexpr unsigned long PRESS_AND_HOLD_LENGTH_MS = 500; // Length of time a key must be held for a long press.

#ifdef DEBUG
// Helper function to write a 16 bit value out as bits for debugging purposes.
void write16AsBits(uint16_t value)
{
  for (int i = 0; i < 8; i++)
  {
    bool b = value & 0x8000;
    Serial.print(b);
    value = value << 1;
  }

  Serial.print(" ");
  for (int i = 0; i < 8; i++)
  {
    bool b = value & 0x8000;
    Serial.print(b);
    value = value << 1;
  }
}

// Helper function to write an 8 bit value out as bits for debugging purposes.
void write8AsBits(uint8_t value)
{
  for (int i = 0; i < 8; i++)
  {
    bool b = value & 0x80;
    Serial.print(b);
    value = value << 1;
  }
}
#endif

ExpanderManager::ExpanderManager(uint8_t address, ButtonEvent buttonHandler)
{
  _mcp = new MCP23017(address);
  _deviceAddress = address;
  _buttonHandler = buttonHandler;
}

/**
 * @brief Get the bit position for a single low bit in a byte.
 *
 * @param value The byte to check, with the active bit set to low and inactive bits set high
 * @return int The position of the single low bit in the byte
 */
int ExpanderManager::GetBitPosition(uint16_t value)
{
  // The entire key detection logic uses low for active so invert the bits
  // to ensure this magic works properly.
  value = ~value;
  // value is a power of two, returned values are {0, 1, ..., 15}
  // This is effectively calculating log2(n) since it's guaranteed there will only ever
  // be one bit set in the value. It's a variation of the method shown
  // at http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog.
  return (((value & 0xAAAAAAAA) != 0) |
          (((value & 0xCCCCCCCC) != 0) << 1) |
          (((value & 0xF0F0F0F0) != 0) << 2) |
          (((value & 0xFF00FF00) != 0) << 3));
}

/**
 * @brief initializes the keyboard matrix.
 *
 */
void ExpanderManager::Init()
{
  _mcp->init();

  _mcp->writeRegister(MCP23017Register::IODIR_A, 0xFF, 0xFF); // All as input
  _mcp->writeRegister(MCP23017Register::GPIO_A, 0xFF, 0xFF);  // Reset all to 1s
  _mcp->writeRegister(MCP23017Register::GPPU_A, 0xFF, 0xFF);  // Turn on pull up resistors

  _currentState = DetectionState::WaitingForPress;
}

/**
 * @brief Determines which button is currently pressed when a row changed interrupt fires.
 *
 */
void ExpanderManager::CheckForButton()
{
  uint16_t buttonStates = _mcp->read();

  // If nothing is pressed then just return
  if (buttonStates == 0xFFFF)
  {
    return;
  }

  // Figure out which button is actually pressed. This assumes only one button
  // will ever be pressed at a given time.
  _activeButton = ExpanderManager::GetBitPosition(buttonStates);

#ifdef DEBUG
  Serial.print("Detected press at: ");
  Serial.print(_activeButton);
  Serial.println();
#endif

  _buttonHandler(ButtonState::Pressed, _deviceAddress, _activeButton);

  // Save when the press event happened for debouncing purposes.
  _lastPressEventTime = millis();

  _currentState = WaitingForRelease;
}

/**
 * @brief Determines when a button was released.
 *
 */
void ExpanderManager::CheckForRelease()
{
  uint16_t buttonStates;

  // Read the row state to see if the button was released. This has the side effect
  // of clearing the interrupt if the triggering pin reset as well.
  buttonStates = _mcp->read();

  // If all the inputs for the row are back to 1s then the button was released
  if (buttonStates == 0xFFFF)
  {
#ifdef DEBUG
    Serial.print("Detected release at: ");
    Serial.print(_activeButton);
    Serial.println();
#endif

    _buttonHandler(ButtonState::Released, _deviceAddress, _activeButton);

    _currentState = WaitingForPress;
  }
}

void ExpanderManager::Loop()
{
  // Unfortunately interrupt-based debouncing as described in the application note for the MCP23017
  // isn't enough to handle key debouncing. This check prevents duplicate key events which are
  // quite common when just relying on the interrupt method.
  if ((millis() - _lastPressEventTime) < DEBOUNCE_TIME_MS)
  {
    return;
  }

  // Fininte state machine for button detection
  switch (_currentState)
  {
  case WaitingForPress:
    CheckForButton();
    break;
  case WaitingForRelease:
    CheckForRelease();
    break;
  }
}
