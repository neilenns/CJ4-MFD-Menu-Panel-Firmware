#pragma once

#include "ExpanderManager.h"

enum MFDevice
{
  kTypeButton = 1,  // 1
  kTypeOutput = 3,  // 3
  kTypeEncoder = 8, // 8
};

// This is the list of recognized commands. These can be commands that can either be sent or received.
// In order to receive, attach a callback function to these events
//
// If you increase this list, make sure to check that the MAXCALLBACKS value
// in CmdMessenger.h is set apropriately
enum MFMessage
{
  kInitModule = 0,          // 0
  kSetModule = 1,           // 1
  kSetPin = 2,              // 2
  kStatus = 5,              // 5
  kEncoderChange = 6,       // 6
  kButtonChange = 7,        // 7
  kGetInfo = 9,             // 9
  kInfo = 10,               // 10
  kSetConfig = 11,          // 11
  kGetConfig = 12,          // 12
  kResetConfig = 13,        // 13
  kSaveConfig = 14,         // 14
  kConfigSaved = 15,        // 15
  kActivateConfig = 16,     // 16
  kConfigActivated = 17,    // 17
  kSetPowerSavingMode = 18, // 18
  kSetName = 19,            // 19
  kGenNewSerial = 20,       // 20
  kTrigger = 23,            // 23
  kResetBoard = 24,         // 24
};

void attachCommandCallbacks();
void generateSerial(bool force);
void HandlerOnButton(uint8_t eventId, uint8_t pin, const __FlashStringHelper *name);
void HandlerOnEncoder(uint8_t eventId, uint8_t pin, const __FlashStringHelper *name);
void loadConfig();
void OnActivateConfig();
void OnButtonPress(ButtonState state, uint8_t deviceAddress, uint8_t button);
void OnGenNewSerial();
void OnGetConfig();
void OnGetInfo();
void OnLEDEvent();
void OnMCP1Interrupt();
void OnMCP2Interrupt();
void OnResetBoard();
void OnSaveConfig();
void OnSetConfig();
void OnSetName();
void OnSetPin();
void OnUnknownCommand();
void readConfig();
void SendOk();
void SetPowerSavingMode(bool state);
void updatePowerSaving();