#pragma once

#include <array>

#include "gb_driver.h"

// Channel 1
#define NR10 0xFF10
#define NR11 0xFF11
#define NR12 0xFF12
#define NR13 0xFF13
#define NR14 0xFF14

// Channel 2
#define NR21 0xFF16
#define NR22 0xFF17
#define NR23 0xFF18
#define NR24 0xFF19

// Channel 3
#define NR30 0xFF1A
#define NR31 0xFF1B
#define NR32 0xFF1C
#define NR33 0xFF1D
#define NR34 0xFF1E
#define WAVE_PATTERN_START 0xFF30
#define WAVE_PATTERN_END 0xFF40

// Channel 4
#define NR41 0xFF20
#define NR42 0xFF21
#define NR43 0xFF22
#define NR44 0xFF23

// Sound Control Registers
#define NR50 0xFF24
#define NR51 0xFF25
#define NR52 0xFF26

#define FREQ_TO_PERIOD(x) (2048 - x) * 4
#define FREQ_CLOCKS 32768 


class APU {
    public:
        APU(GameboyDriver *driver);
        ~APU() = default;

        friend class Channel1;

    public:
        // Handles reads to APU managed high ram registers
        bool regWrite(uint16_t addr, uint8_t data);
        bool regRead(uint16_t addr, uint8_t &val);

        void reset();
        void clock(uint8_t clocks);

    private:
        uint8_t total_clocks;
        uint8_t sample_frequency;

        GameboyDriver *driver;
        std::array<uint8_t, 4> duty_cycles;

        bool isAPUEnabled();
        uint8_t getLeftVolume();
        uint8_t getRightVolume();
    
        // Channel 1
        void clockCh1(uint8_t clocks);
        void resetCh1();
        bool isCh1Enabled();
        uint8_t getCh1Output();
        uint8_t getCh1Duty();
        uint8_t getCh1Volume();
        uint16_t getCh1Frequency();

        uint8_t ch1_pointer;
        uint16_t ch1_timer;
        uint16_t ch1_freq_timer; 

        // Channel 2
        void clockCh2(uint8_t clocks);
        void resetCh2();
        bool isCh2Enabled();
        uint8_t getCh2Output();
        uint8_t getCh2Duty();
        uint8_t getCh2Volume();
        uint16_t getCh2Frequency();

        uint8_t ch2_pointer;
        uint16_t ch2_timer;

        // Channel 3
        void resetCh3();
        bool isCh3Enabled();
        uint16_t getCh3Frequency();

        uint8_t ch3_pointer;
        uint16_t ch3_timer;

        // Channel 4    
        void resetCh4();
        bool isCh4Enabled();

        uint16_t ch4_timer;

    private:
        // APU REGISTERS
        // Channel 1
        uint8_t nr10;
        uint8_t nr11;
        uint8_t nr12;
        uint8_t nr13;
        uint8_t nr14;
        
        // Channel 2
        uint8_t nr21;
        uint8_t nr22;
        uint8_t nr23;
        uint8_t nr24;

        // Channel 3
        uint8_t nr30;
        uint8_t nr31;
        uint8_t nr32;
        uint8_t nr33;
        uint8_t nr34;
        std::array<uint8_t, 0x10> wave_pattern_ram;

        // Channel 4
        uint8_t nr41;
        uint8_t nr42;
        uint8_t nr43;
        uint8_t nr44;

        // Sound Control Registers
        uint8_t nr50;
        uint8_t nr51;
        uint8_t nr52;
};
