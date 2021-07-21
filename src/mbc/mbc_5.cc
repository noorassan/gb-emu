#include "bit_utils.h"
#include "mbc/mbc_5.h"

MBC5::MBC5(uint16_t rom_banks, uint16_t ram_banks) : MBC(rom_banks, ram_banks) {
    rom_bank_lo = 1;
    rom_bank_hi = 0;

    ram_bank = 0;
    ram_enabled = false;
}

bool MBC5::read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) {
    if (addr >= 0x0000 && addr < 0x4000) {
        mapped_addr = addr;
        return true;
    } else if (addr >= 0x4000 && addr < 0x8000) {
        mapped_addr = mapROMAddress(addr, getROMBank());
        return true;
    } else if (addr >= 0xA000 && addr < 0xC000 && ram_enabled) {
        mapped_addr = mapRAMAddress(addr, getRAMBank());
        return true;
    }

    data = 0xFF;
    return false;
}

bool MBC5::write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) {
    if (addr >= 0x0000 && addr < 0x2000) {
        ram_enabled = (data & 0x0F) == 0x0A;
    } else if (addr >= 0x2000 && addr < 0x3000) {
        rom_bank_lo = data;
    } else if (addr >= 0x3000 && addr < 0x4000) {
        rom_bank_hi = data;
    } else if (addr >= 0x4000 && addr < 0x6000) {
        ram_bank = data;
    } else if (addr >= 0xA000 && addr < 0xC000 && ram_enabled) {
        mapped_addr = mapRAMAddress(addr, getRAMBank());
        return true;
    }

    return false;
}

uint16_t MBC5::getRAMBank() {
    return ram_bank & 0x0F; //& (highestOrderBit(ram_banks) - 1);
}

uint16_t MBC5::getROMBank() {
    return (rom_bank_lo | ((rom_bank_hi & 0x01) << 8)); //& (highestOrderBit(ram_banks) - 1);
}