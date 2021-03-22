#include <cstdint>

#include "no_mbc.h"


bool NoMBC::read(uint16_t addr, uint32_t &mapped_addr, bool &rom_read) {
    if (addr >= 0x0000 && addr < 0x8000) {
        mapped_addr = addr;
        rom_read = true;
        return true;
    } else if (addr >= 0xA000 && addr < 0xC000) {
        mapped_addr = addr;
        rom_read = false;
        return true;
    }

    return false;
}

bool NoMBC::write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) {
    if (addr >= 0xA000 && addr < 0xC000) {
        mapped_addr = addr;
        return true;
    }

    return false;
}