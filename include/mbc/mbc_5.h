#pragma once

#include <cstdint>

#include "mbc.h"


class MBC5 : public MBC {
    public:
        MBC5(uint16_t rom_banks, uint16_t ram_banks);
        ~MBC5() = default;
    
    public:
        bool read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override;
        bool write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) override;

    private:
        uint16_t getRAMBank();
        uint16_t getROMBank();

    private:
        uint8_t rom_bank_lo;
        uint8_t rom_bank_hi;

        uint8_t ram_bank;

        bool ram_enabled;
};