#pragma once

#include "mbc/mbc.h"


class NoMBC : public MBC {
    public:
        NoMBC(uint16_t rom_banks, uint8_t ram_banks) : MBC(rom_banks, ram_banks) {}
        ~NoMBC() = default;

    public:
        bool read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override;
        bool write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) override;
};