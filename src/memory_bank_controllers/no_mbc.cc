#include <cstdint>

#include "no_mbc.h"


bool NoMBC::read(uint16_t addr, uint16_t &mapped_addr) {
    if ((addr >= 0x0000 && addr < 0x8000) || 
        (addr >= 0xA000 && addr < 0xC000)) {
        mapped_addr = addr;
        return true;
    }

    return false;
}

bool NoMBC::write(uint16_t addr, uint8_t data, uint16_t &mapped_addr) {
    if (addr >= 0xA000 && addr < 0xC000) {
        mapped_addr = addr;
        return true;
    }

    return false;
}