#include <cstdint>

#include "mbc.h"

class MBC1 : public MemoryBankController {
public:
    MBC1();
    ~MBC1() = default;

public:
    bool read(uint16_t addr, uint16_t &mapped_addr, bool &rom_read) override;
    bool write(uint16_t addr, uint8_t data, uint16_t &mapped_addr) override;

    uint8_t getRAMBank();
    uint8_t getROMBank();

private:
    // Specified by write to 0x0000-0x1FFFF
    // 0xXA enables (X doesn't matter). Anything else disables
    bool ram_enabled;

    // Specified by write to 0x6000-0x7FFF
    // Writing 0x00 -> ROM mode, 0x01 -> RAM mode
    // True = in ROM mode; False = in RAM mode.
    bool rom_mode;

    // Bits 0-4 specified by write to 0x2000-0x3FFF -- write XXXBBBBB (X doesn't matter)
    // Bits 5-6 specified by write to 0x4000-0x5FFF -- write XXXXXXBB (X doesn't matter)
    // If in ROM mode, bits 0-6 specify ROM number & only RAM bank 0 can be accessed.
    // If in RAM mode, bits 0-4 specify ROM number & bits 5-6 specify RAM bank.
    // ROM selection discrepancies:
    //  0 -> Bank 1; 0x20 -> Bank 0x21; 0x40 -> Bank 0x41; 0x60 -> Bank 0x61
    uint8_t bank_specifier;
};