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

ExpanderManager::ExpanderManager(uint8_t address, uint8_t interruptPin, KeyboardEvent interruptHandler, ButtonEvent buttonHandler)
{
  _mcp = new MCP23017(address);
  _interruptPin = interruptPin;
  _interruptHandler = interruptHandler;
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
 * @brief Interrupt handler for when the row changed interrupt fires.
 *
 */
void ExpanderManager::HandleInterrupt()
{
  if (_currentState == WaitingForPress)
  {
    _currentState = DetectionState::PressDetected;
  }
}

/**
 * @brief initializes the keyboard matrix.
 *
 */
void ExpanderManager::Init()
{
  _mcp->init();

  _mcp->writeRegister(MCP23017Register::IODIR_A, 0xFF, 0xFF);  // All as input input
  _mcp->writeRegister(MCP23017Register::GPIO_A, 0xFF, 0xFF);   // Reset all to 1s
  _mcp->writeRegister(MCP23017Register::GPPU_A, 0xFF, 0xFF);   // All have pull-up resistors off for all connected lines, but on for the four unused rows
  _mcp->writeRegister(MCP23017Register::INTCON_A, 0xFF, 0xFF); // Turn interrupts on for all
  _mcp->writeRegister(MCP23017Register::DEFVAL_A, 0xFF, 0xFF); // Default value of 1 for all

  // Attach the Arduino interrupt handler.
  pinMode(_interruptPin, INPUT_PULLUP);
  attachInterrupt(
      digitalPinToInterrupt(_interruptPin), _interruptHandler,
      CHANGE);

  // The order of interrupt startup matters a lot. After the row is initialized for
  // interrupt detection then the state machine is set to WaitingForPress. This ensures
  // in the rare case that an interrupt fires immediately after initialization
  // that the state machine won't miss it.
  _mcp->interruptMode(MCP23017InterruptMode::Or); // Interrupt on one line

  // Enable interrupts
  _mcp->writeRegister(MCP23017Register::GPINTEN_A, 0xFF, 0xFF);

  _mcp->clearInterrupts(); // Clear all interrupts which could come from initialization
  _currentState = DetectionState::WaitingForPress;
}

/**
 * @brief Determines which button is currently pressed when a row changed interrupt fires.
 *
 */
void ExpanderManager::CheckForButton()
{
  uint8_t buttonIntfA, buttonIntfB;
  uint16_t buttonStates;

  // Read the INTF registers to figure out which pin caused the interrupt. INTF is
  // used isntead of GPIO to cover the case of the button bouncing and reading 0 by the
  // time the code gets to read the GPIO pins. INTCAP can't be used either because
  // only INTCAP_A or INTCAP_B updates on an interrupt (depending on which port caused it),
  // and there's no way to clear them.
  _mcp->readRegister(MCP23017Register::INTF_A, buttonIntfA, buttonIntfB);

  // The port A values are the high byte of the row state. Since the INTF registers
  // use a 1 to indicate the pin that fired the interrupt and Arduinos expect a 0
  // on a button press the entire value gets inverted.
  buttonStates = ~((buttonIntfA) << 8 | buttonIntfB);

  _activeButton = ExpanderManager::GetBitPosition(buttonStates);

#ifdef DEBUG
  Serial.print("Detected press at: ");
  Serial.print(_activeButton);
#endif

  // Save when the press event happened so a test can be done on release
  // to look for a press-and-hold on the CLR/DEL key.
  _lastPressEventTime = millis();

  _currentState = WaitingForRelease;
}

/**
 * @brief Determines when a button was released by watching for the row port to reset
 * to all 1s.
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
#endif

    _buttonHandler(ButtonState::Released, _activeButton);

    // Issue 7
    // The order of these two lines is very important. Interrupts get enabled
    // after the state machine is reset to waiting for an interrupt. Otherwise
    // a race condition can (and did!) occur where an interrupt fires and then
    // the state machine resets back to waiting for an interrupt, resulting
    // in the interrupt never getting handled and all further key detection
    // being blocked.
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
    // Nothing to do here, interrupts will handle it
    break;
  case PressDetected:
    CheckForButton();
    break;
  case WaitingForRelease:
    CheckForRelease();
    break;
  }
}
