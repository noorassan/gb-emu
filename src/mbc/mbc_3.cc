#include <algorithm>

#include "mbc/mbc_3.h"

MBC3::MBC3() {
    ram_enabled = false;
    ram_bank = 0;
    rom_bank = 0;
}

bool MBC3::read(uint16_t addr, uint32_t &mapped_addr, bool &rom_read) {
    if (addr >= 0x0000 && addr < 0x4000) {
        mapped_addr = addr;
        rom_read = true;
        return true;
    } else if (addr >= 0x4000 && addr < 0x8000) {
        rom_read = true;
        mapped_addr = (addr & 0x3FFF) + rom_bank * 0x4000;
        return true;
    } else if (addr >= 0xA000 && addr < 0xC000 && ram_enabled) {
        rom_read = false;
        mapped_addr = (addr & 0x1FFF) + ram_bank * 0x2000;
        return true;
    }

    return false;
}

bool MBC3::write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) {
    if (addr >= 0x0000 && addr < 0x2000) {
        ram_enabled = (data & 0x0F) == 0x0A;
    } else if (addr >= 0x2000 && addr < 0x4000) {
        // ROM bank 0 cannot be selected since it is always available
        rom_bank = std::max(1, data & 0xBF);
    } else if (addr >= 0x4000 && addr < 0x6000) {
        ram_bank = data;
    } else if (addr >= 0x6000 && addr < 0x8000) {
        // TODO: Add RTC clock
    } else if (addr >= 0xA000 && addr < 0xC000) {
        mapped_addr = (addr & 0x1FFF) + ram_bank * 0x2000;
        return true;
    }

    return false;
}