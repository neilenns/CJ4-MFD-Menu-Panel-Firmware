#pragma once

#include <Arduino.h>

// Physical pins for the LED driver.
static constexpr uint8_t LED_SDB_PIN = 6;  // Arduino pin connected to SDB on the LED driver.
static constexpr uint8_t LED_INTB_PIN = 1; // Arduino pin connected to to INTB on the LED driver.

// Virtual pins for one-off MobiFlight "modules". The virtual pins created
// for the MCP expanders start at 100, so other random virtual pins work
// their way down from there.
static constexpr uint8_t BRIGHTNESS_PIN = 99; // Brightness control for the LED driver.

// Physical pins for the five-way button.
static constexpr uint8_t PIN_LEFT = A4;
static constexpr uint8_t PIN_UP = A3;
static constexpr uint8_t PIN_RIGHT = A2;
static constexpr uint8_t PIN_DOWN = A1;
static constexpr uint8_t PIN_CTR = A0;

// Physical pins for the dual encoder.
static constexpr uint8_t ENCODER_TYPE = 0;
static constexpr uint8_t PIN_A = 8;
static constexpr uint8_t PIN_A_PRIME = 9;
static constexpr uint8_t PIN_B = 5;
static constexpr uint8_t PIN_B_PRIME = 10;