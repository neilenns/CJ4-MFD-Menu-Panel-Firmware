#pragma once

#include <Arduino.h>

// The methods for storing all this data in PROGMEM comes from
// https://www.arduino.cc/reference/en/language/variables/utilities/progmem/

namespace ExpanderButtonNames
{
    static constexpr uint8_t ButtonLUT[] PROGMEM = {
        20, // DATA
        255,
        255,
        255,
        255,
        0,  // RADAR_MENU
        17, // MEM_1
        1,  // LWR_MENU
        255,
        255,
        2,  // UPR_MENU
        3,  // ESC
        4,  // DATABASE
        5,  // NAV_DATA
        6,  // CAS_PAGE
        7,  // CHART
        8,  // CRSR
        9,  // PASS_BRIEF
        10, // SYS
        11, // CKLIST
        19, // MEM_3
        12, // TFC
        13, // TERR_WX
        14, // ENG
        255,
        255,
        15, // ZOOM_PLUS
        16, // ZOOM_MINUS
        18, // MEM_2
        255,
        255,
        255};

    static constexpr uint8_t MaxNameLength = 11;

    static constexpr char SW1[] PROGMEM = "RADAR_MENU";
    static constexpr char SW2[] PROGMEM = "LWR_MENU";
    static constexpr char SW3[] PROGMEM = "UPR_MENU";
    static constexpr char SW4[] PROGMEM = "ESC";
    static constexpr char SW5[] PROGMEM = "DATABASE";
    static constexpr char SW6[] PROGMEM = "NAV_DATA";
    static constexpr char SW7[] PROGMEM = "CAS_PAGE";
    static constexpr char SW8[] PROGMEM = "CHART";
    static constexpr char SW9[] PROGMEM = "CRSR";
    static constexpr char SW10[] PROGMEM = "PASS_BRIEF";
    static constexpr char SW11[] PROGMEM = "SYS";
    static constexpr char SW12[] PROGMEM = "CKLIST";
    static constexpr char SW13[] PROGMEM = "TFC";
    static constexpr char SW14[] PROGMEM = "TERR_WX";
    static constexpr char SW15[] PROGMEM = "ENG";
    static constexpr char SW16[] PROGMEM = "ZOOM_PLUS";
    static constexpr char SW17[] PROGMEM = "ZOOM_MINUS";
    static constexpr char SW18[] PROGMEM = "MEM_1";
    static constexpr char SW19[] PROGMEM = "MEM_2";
    static constexpr char SW20[] PROGMEM = "MEM_3";
    static constexpr char SW21[] PROGMEM = "DATA";
    static constexpr char SW22[] PROGMEM = "MEM_1_LONG";
    static constexpr char SW23[] PROGMEM = "MEM_2_LONG";
    static constexpr char SW24[] PROGMEM = "MEM_3_LONG";
    static constexpr char SW25[] PROGMEM = "DATA_LONG";

    static constexpr uint8_t ButtonCount = 25;

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
        SW24,
        SW25};
}