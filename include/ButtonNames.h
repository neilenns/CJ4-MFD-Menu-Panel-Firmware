#pragma once

#include <Arduino.h>

// The methods for storing all this data in PROGMEM comes from
// https://www.arduino.cc/reference/en/language/variables/utilities/progmem/

namespace ButtonNames
{
    constexpr uint8_t ButtonLUT[] PROGMEM = {
        255,
        255,
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        255,
        255,
        255,
        255,
        7,
        8,
        9,
        255,
        255,
        10,
        11,
        12,
        255,
        255,
        255,
        13,
        14,
        15,
        16,
        17,
        18,
        19,
        20};

    constexpr uint8_t MaxNameLength = 11;

    constexpr char SW1[] PROGMEM = "UPR_MENU";
    constexpr char SW2[] PROGMEM = "ESC";
    constexpr char SW3[] PROGMEM = "DATABASE";
    constexpr char SW4[] PROGMEM = "NAV_DATA";
    constexpr char SW5[] PROGMEM = "CAS_PAGE";
    constexpr char SW6[] PROGMEM = "CHART";
    constexpr char SW7[] PROGMEM = "E_BTN";
    constexpr char SW8[] PROGMEM = "RADAR_MENU";
    constexpr char SW9[] PROGMEM = "MEM_1";
    constexpr char SW10[] PROGMEM = "LWR_MENU";
    constexpr char SW11[] PROGMEM = "ZOOM_PLUS";
    constexpr char SW12[] PROGMEM = "ZOOM_MINUS";
    constexpr char SW13[] PROGMEM = "MEM_2";
    constexpr char SW14[] PROGMEM = "CRSR";
    constexpr char SW15[] PROGMEM = "PASS_BRIEF";
    constexpr char SW16[] PROGMEM = "SYS";
    constexpr char SW17[] PROGMEM = "CKLIST";
    constexpr char SW18[] PROGMEM = "MEM_3";
    constexpr char SW19[] PROGMEM = "TFC";
    constexpr char SW20[] PROGMEM = "TERR_WX";
    constexpr char SW21[] PROGMEM = "ENG";

    constexpr uint8_t ButtonCount = 21;

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
        SW21};
}