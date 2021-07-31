#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "mbc/mbc.h"


class MBC2 : public MBC {
    public:
        MBC2(uint16_t rom_banks, uint8_t ram_banks);
        ~MBC2() = default;

    public:
        bool read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override;
        bool write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) override;
        
        void saveRAM(std::ofstream &ofs) override;
        void loadRAM(std::ifstream &ifs) override;

    private:
        bool ram_enabled;
        uint8_t rom_bank;

        std::array<uint8_t, 0x200> ram;
};