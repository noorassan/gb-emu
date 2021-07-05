#include <cstdint>

#include "mbc/mbc.h"


class MBC1 : public MBC {
    public:
        MBC1(uint16_t rom_banks, uint8_t ram_banks);
        ~MBC1() = default;

    public:
        bool read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override;
        bool write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) override;

    private:
        uint8_t getRAMBank();
        uint8_t getROMBank();

        // Specified by write to 0x0000-0x1FFFF
        // 0xXA enables (X doesn't matter). Anything else disables
        bool ram_enabled;

        // Specified by write to 0x6000-0x7FFF
        // Writing 0x00 -> ROM mode, 0x01 -> RAM mode
        bool ram_mode;

        // Bits 0-4 specified by write to 0x2000-0x3FFF -- write XXXBBBBB (X doesn't matter)
        uint8_t bank_reg_1;

        // Bits 5-6 specified by write to 0x4000-0x5FFF -- write XXXXXXBB (X doesn't matter)
        uint8_t bank_reg_2;

};