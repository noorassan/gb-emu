#pragma once

#include <array>

#include "gb_driver.h"
#include "apu/apu_addrs.h"
#include "apu/channel_1.h"
#include "apu/channel_2.h"
#include "apu/channel_3.h"
#include "apu/channel_4.h"


class APU {
    public:
        APU(GameboyDriver *driver);
        ~APU() = default;

    public:
        // Handles reads to APU managed high ram registers
        bool regWrite(uint16_t addr, uint8_t data);
        bool regRead(uint16_t addr, uint8_t &val);

        void reset();
        void clock(uint8_t clocks);

    private:
        uint32_t clocks_to_sample;
        uint32_t sample_frequency;

        GameboyDriver *driver;
        std::array<uint8_t, 4> duty_cycles;

        bool isAPUEnabled();
        uint8_t getLeftVolume();
        uint8_t getRightVolume();

        Channel1 ch1;
        Channel2 ch2;
        Channel3 ch3;
        Channel4 ch4;

    private:
        // Sound Control Registers
        uint8_t nr50;
        uint8_t nr51;
        uint8_t nr52;
};
