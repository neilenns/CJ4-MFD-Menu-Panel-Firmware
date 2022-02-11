#pragma once

#include <Arduino.h>

// The methods for storing all this data in PROGMEM comes from
// https://www.arduino.cc/reference/en/language/variables/utilities/progmem/

namespace ExpanderButtonNames
{
    constexpr uint8_t ButtonLUT[] PROGMEM = {
        0, // E_BTN
        255,
        255,
        255,
        255,
        1,  // RADAR_MENU
        18, // MEM_1
        2,  // LWR_MENU
        255,
        255,
        3,  // UPR_MENU
        4,  // ESC
        5,  // DATABASE
        6,  // NAV_DATA
        7,  // CAS_PAGE
        8,  // CHART
        9,  // CRSR
        10, // PASS_BRIEF
        11, // SYS
        12, // CKLIST
        20, // MEM_3
        13, // TFC
        14, // TERR_WX
        15, // ENG
        255,
        255,
        16, // ZOOM_PLUS
        17, // ZOOM_MINUS
        19, // MEM_2
        255,
        255,
        255};

    constexpr uint8_t MaxNameLength = 11;

    constexpr char SW1[] PROGMEM = "E_BTN";
    constexpr char SW2[] PROGMEM = "RADAR_MENU";
    constexpr char SW3[] PROGMEM = "LWR_MENU";
    constexpr char SW4[] PROGMEM = "UPR_MENU";
    constexpr char SW5[] PROGMEM = "ESC";
    constexpr char SW6[] PROGMEM = "DATABASE";
    constexpr char SW7[] PROGMEM = "NAV_DATA";
    constexpr char SW8[] PROGMEM = "CAS_PAGE";
    constexpr char SW9[] PROGMEM = "CHART";
    constexpr char SW10[] PROGMEM = "CRSR";
    constexpr char SW11[] PROGMEM = "PASS_BRIEF";
    constexpr char SW12[] PROGMEM = "SYS";
    constexpr char SW13[] PROGMEM = "CKLIST";
    constexpr char SW14[] PROGMEM = "TFC";
    constexpr char SW15[] PROGMEM = "TERR_WX";
    constexpr char SW16[] PROGMEM = "ENG";
    constexpr char SW17[] PROGMEM = "ZOOM_PLUS";
    constexpr char SW18[] PROGMEM = "ZOOM_MINUS";
    constexpr char SW19[] PROGMEM = "MEM_1";
    constexpr char SW20[] PROGMEM = "MEM_2";
    constexpr char SW21[] PROGMEM = "MEM_3";
    constexpr char SW22[] PROGMEM = "MEM_1_LONG";
    constexpr char SW23[] PROGMEM = "MEM_2_LONG";
    constexpr char SW24[] PROGMEM = "MEM_3_LONG";

    constexpr uint8_t ButtonCount = 24;

    const char *const Names[ButtonCount] PROGMEM = {
        SW1,
        SW2,
        SW3,
        SW4,
        SW5,
        SW6,
        SW7,
        SW8,
        SW9,
        SW10,
        SW11,
        SW12,
        SW13,
        SW14,
        SW15,
        SW16,
        SW17,
        SW18,
        SW19,
        SW20,
        SW21,
        SW22,
        SW23,
        SW24};
}