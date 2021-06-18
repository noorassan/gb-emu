#include <cstdint>

#include "mbc/mbc.h"


class MBC3 : public MemoryBankController {
public:
    MBC3();
    ~MBC3() = default;

public:
    bool read(uint16_t addr, uint32_t &mapped_addr, bool &rom_read) override;
    bool write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) override;

private:
    // Specified by write to 0x0000-0x1FFFF
    // 0xXA enables (X doesn't matter). Anything else disables
    bool ram_enabled;

    uint8_t rom_bank;
    uint8_t ram_bank;
};