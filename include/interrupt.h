#pragma once

// interrupt flag/enable register locations
#define IE 0xFFFF
#define IF 0xFF0F

// interrupt start addresses
#define VBLANK_A 0x0040
#define LCDS_A 0x0048
#define TIMER_A 0x0050
#define SERIAL_A 0x0058
#define JOYPAD_A 0x0060

enum INTERRUPT {
    V_BLANK = 1 << 0,
    LCD_STAT = 1 << 1,
    TIMER = 1 << 2,
    SERIAL = 1 << 3,
    JOYPAD = 1 << 4,
};