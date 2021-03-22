#include "mbc_1.h"

MBC1::MBC1() {
    ram_enabled = false;
    rom_mode = true;

    bank_specifier = 0;
}

bool MBC1::read(uint16_t addr, uint16_t &mapped_addr, bool &rom_read) {
    if (addr >= 0x0000 && addr < 0x4000) {
        mapped_addr = addr;
        rom_read = true;
        return true;
    } else if (addr >= 0x4000 && addr < 0x8000) {
        uint8_t bank = getROMBank();
        rom_read = true;
        mapped_addr = addr + (bank - 1) * 0x4000;
        return true;
    } else if (addr >= 0xA000 && addr < 0xC000 && ram_enabled) {
        uint8_t bank = getRAMBank();
        rom_read = false;
        mapped_addr = addr + (bank) * 0x2000;
        return true;
    }

    return false;
}

bool MBC1::write(uint16_t addr, uint8_t data, uint16_t &mapped_addr) {
    if (addr >= 0x0000 && addr < 0x2000) {
        ram_enabled = (data & 0x0F) == 0x0A;
    } else if (addr >= 0x2000 && addr < 0x4000) {
        bank_specifier &= 0xE0; // clear lower 5 bits
        bank_specifier |= (data & 0x1F); // set lower 5 bits
    } else if (addr >= 0x4000 && addr < 0x6000) {
        bank_specifier &= 0x1F; // clear upper 3 bits
        bank_specifier |= (data & 0x03) << 5; // set bits 5 & 6
    } else if (addr >= 0x6000 && addr < 0x8000) {
        rom_mode = (data == 0x00);
    } else if (addr >= 0xA000 && addr < 0xC000) {
        uint8_t bank = getRAMBank();
        mapped_addr = addr + (bank) * 0x2000;
        return true;
    }

    return false;
}

uint8_t MBC1::getROMBank() {
    uint8_t val;
    if (rom_mode) {
        val = bank_specifier & 0x7F;
    } else {
        val = bank_specifier & 0x1F;
    }

    //  0 -> Bank 1; 0x20 -> Bank 0x21; 0x40 -> Bank 0x41; 0x60 -> Bank 0x61
    if (val % 0x20 == 0) {
        return val + 1;
    } else {
        return val;
    }
}

uint8_t MBC1::getRAMBank() {
    if (rom_mode) {
        return 0;
    } else {
        return (bank_specifier >> 5) & 0x03;
    }
}