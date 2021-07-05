#include "mbc/no_mbc.h"

bool NoMBC::read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) {
    if (addr >= 0x0000 && addr < 0x8000) {
        mapped_addr = addr;
        return true;
    } else if (addr >= 0xA000 && addr < 0xC000) {
        mapped_addr = addr & 0x1FFF;
        return true;
    }

    data = 0x00;
    return false;
}

bool NoMBC::write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) {
    if (addr >= 0xA000 && addr < 0xC000) {
        mapped_addr = addr & 0x1FFF;
        return true;
    }

    return false;
}