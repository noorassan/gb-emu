#pragma once

#include <chrono>
#include <cstdint>

#include "mbc/mbc.h"


class MBC3 : public MBC {
    public:
        MBC3(uint16_t rom_banks, uint8_t ram_banks);
        ~MBC3() = default;

    public:
        bool read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override;
        bool write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) override;

    private:
        uint8_t getRTCData();
        void setRTCData(uint8_t data);

    private:
        // Specified by write to 0x0000-0x1FFFF
        // 0xXA enables (X doesn't matter). Anything else disables
        bool ram_rtc_enabled;

        uint8_t rom_bank;
        uint8_t ram_rtc_bank;

        uint8_t last_latch_write;

        // RTC registers
        uint8_t rtc_s;
        uint8_t rtc_m;
        uint8_t rtc_h;
        uint8_t rtc_dl;
        uint8_t rtc_dh;

};