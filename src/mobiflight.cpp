#include <Arduino.h>
#include <Wire.h>

#include "CmdMessenger.h"
#include "ExpanderButtonNames.h"
#include "ExpanderManager.h"
#include "LEDMatrix.h"
#include "MFButton.h"
#include "MFEEPROM.h"
#include "MFEncoder.h"
#include "mobiflight.h"
#include "PinAssignments.h"

// The build version comes from an environment variable.
#define STRINGIZER(arg) #arg
#define STR_VALUE(arg) STRINGIZER(arg)
#define VERSION STR_VALUE(BUILD_VERSION)

// MobiFlight expects a board name, type, and serial number to come from the board
// when requested. The serial number is stored in flash. The board type
// and name are fixed and come from Board.h.
static constexpr uint8_t MEM_OFFSET_SERIAL = 0;
static constexpr uint8_t MEM_LEN_SERIAL = 11;
char serial[MEM_LEN_SERIAL];

// I2C Addresses for the IO expanders.
static constexpr uint8_t MCP1_I2C_ADDRESS = 0x20; // Address for first MCP23017.
static constexpr uint8_t MCP2_I2C_ADDRESS = 0x21; // Address for second MCP23017.

// Time durations.
static constexpr unsigned long POWER_SAVING_TIME_SECS = 60 * 60; // Inactivity timeout for LEDs. One hour (60 minutes * 60 seconds).
static constexpr unsigned long PRESS_AND_HOLD_LENGTH_MS = 500;   // Length of time a key must be held for a long press.
static constexpr unsigned long BUTTON_DEBOUNCE_LENGTH_MS = 10;   // Number of milliseconds between checking for button presses.

// MobiFlight-style devices.
static constexpr uint8_t MAX_BUTTONS = 5;
MFButton buttons[MAX_BUTTONS];

static constexpr uint8_t MAX_ENCODERS = 2;
MFEncoder encoders[MAX_ENCODERS];

// State variables.
unsigned long lastButtonPress = 0;
unsigned long lastButtonUpdate = 0;
auto powerSavingMode = false;

// Communication & device controller variables.
CmdMessenger cmdMessenger = CmdMessenger(Serial);
MFEEPROM MFeeprom;
ExpanderManager mcp1(MCP1_I2C_ADDRESS, OnButtonPress);
ExpanderManager mcp2(MCP2_I2C_ADDRESS, OnButtonPress);
LEDMatrix ledMatrix(ADDR::GND, ADDR::GND, LED_SDB_PIN, LED_INTB_PIN, OnLEDEvent);

/**
 * @brief Registers callbacks for all supported MobiFlight commands.
 *
 */
void attachCommandCallbacks()
{
  // Attach callback methods
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(MFMessage::kSetPin, OnSetPin);
  cmdMessenger.attach(MFMessage::kGetInfo, OnGetInfo);
  cmdMessenger.attach(MFMessage::kGetConfig, OnGetConfig);
  cmdMessenger.attach(MFMessage::kSetConfig, OnSetConfig);
  cmdMessenger.attach(MFMessage::kResetConfig, SendOk);
  cmdMessenger.attach(MFMessage::kSaveConfig, OnSaveConfig);
  cmdMessenger.attach(MFMessage::kActivateConfig, OnActivateConfig);
  cmdMessenger.attach(MFMessage::kSetName, OnSetName);
  cmdMessenger.attach(MFMessage::kGenNewSerial, OnGenNewSerial);
  cmdMessenger.attach(MFMessage::kTrigger, SendOk);
  cmdMessenger.attach(MFMessage::kResetBoard, OnResetBoard);
#ifdef DEBUG
  cmdMessenger.attach(MFMessage::kGenerateConfig, OnGenerateConfig);
#endif
}

/**
 * @brief Handles an interrupt from the LEDMatrix.
 *
 */
void OnLEDEvent()
{
  ledMatrix.HandleInterrupt();
}

/**
 * @brief General callback to simply respond OK to the desktop app for unsupported commands.
 *
 */
void SendOk()
{
  cmdMessenger.sendCmd(MFMessage::kStatus, F("OK"));
}

/**
 * @brief Callback for the MobiFlight event. This doesn't have to do anything so just report success.
 *
 */
void OnSaveConfig()
{
  cmdMessenger.sendCmd(MFMessage::kConfigSaved, F("OK"));
}

/**
 * @brief Callback for the MobiFlight event. This doesn't have to do anything so just report success.
 *
 */
void OnActivateConfig()
{
  cmdMessenger.sendCmd(MFMessage::kConfigActivated, F("OK"));
}

/**
 * @brief Loads or generates a new board serial number. Sends a kConfigActivated
 * message to MobiFlight for compatibility purposes.
 *
 */
void OnResetBoard()
{
  generateSerial(false);

  // This is required to maintain compatibility with the standard Mobiflight firmware
  // which eventually activates the config when resetting the board.
  OnActivateConfig();
}

/**
 * @brief Loads the board serial number from EEPROM and generates a new one if force is set to true
 * or no serial number was present in EEPROM.
 *
 * @param force True if a new serial number should be created even if one already exists.
 */
void generateSerial(bool force)
{
  MFeeprom.read_block(MEM_OFFSET_SERIAL, serial, MEM_LEN_SERIAL);
  if (!force && serial[0] == 'S' && serial[1] == 'N')
    return;
  randomSeed(analogRead(0));
  sprintf(serial, "SN-%03x-", (unsigned int)random(4095));
  sprintf(&serial[7], "%03x", (unsigned int)random(4095));
  MFeeprom.write_block(MEM_OFFSET_SERIAL, serial, MEM_LEN_SERIAL);
}

/**
 * @brief Callback for handling a button press from a connected MCP.
 *
 * @param state State of the button (pressed or released).
 * @param deviceAddress The I2C address of the MCP that detected the button event.
 * @param button The index of the button pressed on the MCP.
 */
void OnButtonPress(ButtonState state, uint8_t deviceAddress, uint8_t button)
{
  auto isLongPress = false;

  // If the button was pushed on the second MCP then its button address
  // needs to have 16 added to it before doing the button name lookup.
  if (deviceAddress == MCP2_I2C_ADDRESS)
  {
    button += 16;
  }

#ifdef DEBUG
  Serial.print("Button: ");
  Serial.println(button);
#endif

  // The data button and three mem buttons only send release events, and they are
  // either regular or long press.
  // 0 is DATA
  // 6 is MEM_1
  // 20 is MEM_3
  // 28 is MEM_2
  if (((button == 0) || (button == 6) || (button == 20) || (button == 28)))
  {
    // On the press event for these special case buttons just remember
    // when the press happened so the length of the press can be calculated
    // on release.
    if (state == ButtonState::Pressed)
    {
      lastButtonPress = millis();
      return;
    }

    // Check for a long press when released.
    if ((millis() - lastButtonPress) > PRESS_AND_HOLD_LENGTH_MS)
    {
      isLongPress = true;
    }
  }

  // The keyboard matrix provides a button location that has to
  // be mapped to a button name to send the correct event to MobiFlight.
  // The button names are in a 1D array and the pin mapping on the MCPs is
  // sparse so a lookup table is used to get the correct index into the name
  // array for a given pin on the MCP.
  auto index = pgm_read_byte(&(ExpanderButtonNames::ButtonLUT[button]));

  // If the lookup table returns 255 then it's a pin that shouldn't
  // ever fire because it's not connected to anything.
  if (index == 255)
  {
    cmdMessenger.sendCmd(kStatus, "Pin isn't a valid button");
    return;
  }

  // The way the names are stored in the list the long press
  // button names are four farther down from the short press
  // names.
  if (isLongPress)
  {
    index += 4;
  }

  // Get the button name from flash using the index.
  char buttonName[ExpanderButtonNames::MaxNameLength] = "";
  strcpy_P(buttonName, (char *)pgm_read_word(&(ExpanderButtonNames::Names[index])));

  // Send the button name and state to MobiFlight.
  cmdMessenger.sendCmdStart(MFMessage::kButtonChange);
  cmdMessenger.sendCmdArg(buttonName);
  cmdMessenger.sendCmdArg(state);
  cmdMessenger.sendCmdEnd();

  lastButtonPress = millis();
}

/**
 * @brief Callback for setting the board configuration. Since the board configuration is fixed
 * any requests from MobiFlight to set the config are simply ignored and a remaining byte count of
 * 512 is sent back to keep the desktop app happy.
 *
 */
void OnSetConfig()
{
  cmdMessenger.sendCmd(MFMessage::kStatus, 512);
}

/**
 * @brief Callback for unknown commands.
 *
 */
void OnUnknownCommand()
{
  cmdMessenger.sendCmd(MFMessage::kStatus, F("n/a"));
}

/**
 * @brief Callback for sending the board information to MobiFlight.
 *
 */
void OnGetInfo()
{
  cmdMessenger.sendCmdStart(MFMessage::kInfo);
  cmdMessenger.sendCmdArg(F("CJ4 MFD panel"));
  cmdMessenger.sendCmdArg(F("CJ4 MFD panel"));
  cmdMessenger.sendCmdArg(serial);
  cmdMessenger.sendCmdArg(VERSION);
  cmdMessenger.sendCmdEnd();
}

#ifdef DEBUG
/**
 * @brief Generates the configuration string so it can be copied and pasted as a hardcoded string
 * in OnGetConfig().
 */
void OnGenerateConfig()
{
  char singleModule[20] = "";
  char pinName[ExpanderButtonNames::MaxNameLength] = "";

  cmdMessenger.sendCmdStart(MFMessage::kInfo);
  cmdMessenger.sendFieldSeparator();

  // Send configuration for all the buttons. The virtual pins for the two MCP
  // expansions start at 100 to avoid overlapping with the standard Arduino pins.
  for (auto i = 0; i < ExpanderButtonNames::ButtonCount; i++)
  {
    // Get the pin name from flash
    strcpy_P(pinName, (char *)pgm_read_word(&(ExpanderButtonNames::Names[i])));

    snprintf(singleModule, 20, "%i.%i.%s:", MFDevice::kTypeButton, i + 100, pinName);
    cmdMessenger.sendArg(singleModule);
  }

  // Send configuration for the LED brightness output. 99 is the
  // value of BRIGHTNESS_PIN.
  cmdMessenger.sendArg(F("3.99.Brightness:"));

  // Send configuration five-way controller. The pin numbers are defined in
  // PinAssignments.h but to save flash everything is brute force hard coded
  // here.
  cmdMessenger.sendArg(F("1.20.RIGHT:"));
  cmdMessenger.sendArg(F("1.22.LEFT:"));
  cmdMessenger.sendArg(F("1.21.UP:"));
  cmdMessenger.sendArg(F("1.19.DOWN:"));
  cmdMessenger.sendArg(F("1.18.CTR:"));

  // Send configuration for the dual encoders. The 1 is the encoder type.
  cmdMessenger.sendArg(F("8.8.5.1.ENC_1:"));
  cmdMessenger.sendArg(F("8.9.10.1.ENC_2:"));

  cmdMessenger.sendCmdEnd();
}
#endif

/**
 * @brief Callback for sending module configuration to MobiFlight.
 * The module configuration is generated on the fly rather than being stored in EEPROM.
 *
 */
void OnGetConfig()
{
  Serial.println("10,1.100.RADAR_MENU:1.101.LWR_MENU:1.102.UPR_MENU:1.103.ESC:1.104.DATABASE:1.105.NAV_DATA:1.106.CAS_PAGE:1.107.CHART:1.108.CRSR:1.109.PASS_BRIEF:1.110.SYS:1.111.CKLIST:1.112.TFC:1.113.TERR_WX:1.114.ENG:1.115.ZOOM_PLUS:1.116.ZOOM_MINUS:1.117.MEM_1:1.118.MEM_2:1.119.MEM_3:1.120.DATA:1.121.MEM_1_LONG:1.122.MEM_2_LONG:1.123.MEM_3_LONG:1.124.DATA_LONG:3.99.Brightness:1.20.RIGHT:1.22.LEFT:1.21.UP:1.19.DOWN:1.18.CTR:8.8.5.2.ENC_1:8.9.10.2.ENC_2:;");
}

/**
 * @brief Callback for MobiFlight LED output commands.
 *
 */
void OnSetPin()
{
  // Read led state argument, interpret string as boolean.
  auto pin = cmdMessenger.readInt16Arg();
  auto state = cmdMessenger.readInt16Arg();

  // The brightness virtual pin is 69
  if (pin == BRIGHTNESS_PIN)
  {
    cmdMessenger.sendCmd(kStatus, "OK");
    ledMatrix.SetBrightness(state);
    ledMatrix.SetPowerSaveMode(false);
  }
}

/**
 * @brief Generates a new serial number for the board and stores it in EEPROM.
 *
 */
void OnGenNewSerial()
{
  generateSerial(true);
  cmdMessenger.sendCmdStart(MFMessage::kInfo);
  cmdMessenger.sendCmdArg(serial);
  cmdMessenger.sendCmdEnd();
}

/**
 * @brief Stubbed out method to accept the name argument then discard it. The name
 * is actually hardcoded in the firmware.
 *
 */
void OnSetName()
{
  cmdMessenger.readStringArg();
  cmdMessenger.sendCmdStart(MFMessage::kStatus);
  cmdMessenger.sendCmdArg(F("CJ4 MFD panel"));
  cmdMessenger.sendCmdEnd();
}

/**
 * @brief Checks to see if power saving mode should be enabled or disabled
 * based on the last time a key was pressed.
 *
 */
void CheckForPowerSave()
{
  if (!powerSavingMode && ((millis() - lastButtonPress) > (POWER_SAVING_TIME_SECS * 1000)))
  {
    powerSavingMode = true;
    ledMatrix.SetPowerSaveMode(true);
  }
  else if (powerSavingMode && ((millis() - lastButtonPress) < (POWER_SAVING_TIME_SECS * 1000)))
  {
    ledMatrix.SetPowerSaveMode(false);
    powerSavingMode = false;
  }
}

/**
 * @brief Creates the MobiFlight-style buttons and encoders.
 *
 */
void AddMFDevices()
{
  buttons[0] = MFButton(PIN_LEFT, F("LEFT"));
  buttons[1] = MFButton(PIN_RIGHT, F("RIGHT"));
  buttons[2] = MFButton(PIN_UP, F("UP"));
  buttons[3] = MFButton(PIN_DOWN, F("DOWN"));
  buttons[4] = MFButton(PIN_CTR, F("CTR"));
  MFButton::AttachHandler(HandlerOnButton);

  encoders[0] = MFEncoder(PIN_A, PIN_B, 2, F("ENC_1"));
  encoders[1] = MFEncoder(PIN_A_PRIME, PIN_B_PRIME, 2, F("ENC_2"));
  MFEncoder::attachHandler(HandlerOnEncoder);
}

/**
 * @brief Handles events from MobiFlight-style buttons.
 *
 * @param eventId Whether the event is OnPress or OnRelease.
 * @param pin The button pin that fired the event.
 * @param name The name of the button that fired the event.
 */
void HandlerOnButton(uint8_t eventId, uint8_t pin, const __FlashStringHelper *name)
{
  cmdMessenger.sendCmdStart(MFMessage::kButtonChange);
  cmdMessenger.sendCmdArg(name);
  cmdMessenger.sendCmdArg(eventId);
  cmdMessenger.sendCmdEnd();

  lastButtonPress = millis();
};

/**
 * @brief Handles events from MF-style encoders
 *
 * @param eventId
 * @param pin The encoder pin that fired the event.
 * @param name The name of the encoder that fired the event.
 */
void HandlerOnEncoder(uint8_t eventId, uint8_t pin, const __FlashStringHelper *name)
{
  cmdMessenger.sendCmdStart(MFMessage::kEncoderChange);
  cmdMessenger.sendCmdArg(name);
  cmdMessenger.sendCmdArg(eventId);
  cmdMessenger.sendCmdEnd();
};

/**
 * @brief Loops through the MobiFlight-style buttons to check for button events.
 *
 */
void ReadButtons()
{
  for (auto i = 0; i != MAX_BUTTONS; i++)
  {
    buttons[i].Update();
  }
}

/**
 * @brief Loops through MobiFlight-style encoders to check for encoder events.
 *
 */
void ReadEncoders()
{
  for (auto i = 0; i != MAX_ENCODERS; i++)
  {
    encoders[i].Update();
  }
}

/**
 * @brief Arduino initialization method.
 *
 */
void setup()
{
  MFeeprom.init();
  Wire.begin();
  Wire.setClock(400000);
  Serial.begin(115200);

  attachCommandCallbacks();
  cmdMessenger.printLfCr();

  OnResetBoard();
  AddMFDevices();
  mcp1.Init();
  mcp2.Init();
  ledMatrix.Init();

  lastButtonPress = millis();
  lastButtonUpdate = millis();
}

/**
 * @brief Arduino application loop.
 *
 */
void loop()
{
  cmdMessenger.feedinSerialData();

  // Buttons aren't checked every pass through the loop to
  // handle button bouncing.
  if (millis() - lastButtonUpdate >= BUTTON_DEBOUNCE_LENGTH_MS)
  {
    mcp1.Loop();
    mcp2.Loop();
    ReadButtons();
    ReadEncoders();
    lastButtonUpdate = millis();
  }

  CheckForPowerSave();
  ledMatrix.Loop();
}
