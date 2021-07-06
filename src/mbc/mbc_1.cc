#include "bit_utils.h"
#include "mbc/mbc_1.h"

MBC1::MBC1(uint16_t rom_banks, uint8_t ram_banks) : MBC(rom_banks, ram_banks) {
    ram_enabled = false;
    ram_mode = false;

    bank_reg_1 = 0x01;
    bank_reg_2 = 0x00;
}

bool MBC1::read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) {
    if (addr >= 0x0000 && addr < 0x4000) {
        mapped_addr = addr;
        return true;
    } else if (addr >= 0x4000 && addr < 0x8000) {
        mapped_addr = mapROMAddress(addr, getROMBank());
        return true;
    } else if (addr >= 0xA000 && addr < 0xC000 && ram_enabled) {
        if (ram_enabled) {
            mapped_addr = mapRAMAddress(addr, getRAMBank());
            return true;
        }
    }

    data = 0xFF;
    return false;
}

bool MBC1::write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) {
    if (addr >= 0x0000 && addr < 0x2000) {
        ram_enabled = (data & 0x0F) == 0x0A;
    } else if (addr >= 0x2000 && addr < 0x4000) {
        //  0 -> Bank 1; 0x20 -> Bank 0x21; 0x40 -> Bank 0x41; 0x60 -> Bank 0x61
        if (data % 0x20 == 0) data++;
        bank_reg_1 = data & 0x1F;
    } else if (addr >= 0x4000 && addr < 0x6000) {
        bank_reg_2 = data & 0x03;
    } else if (addr >= 0x6000 && addr < 0x8000) {
        ram_mode = data;
    } else if (addr >= 0xA000 && addr < 0xC000) {
        if (ram_enabled) {
            mapped_addr = mapRAMAddress(addr, getRAMBank());
            return true;
        }
    }

    return false;
}

uint8_t MBC1::getROMBank() {
    uint8_t bank;
    if (ram_mode) {
        bank = bank_reg_1;
    } else {
        bank = bank_reg_1 + (bank_reg_2 << 5);
    }

    // masked to fit number of rom banks
    bank &= (highestOrderBit(rom_banks) - 1);
    return bank;
}

uint8_t MBC1::getRAMBank() {
    if (ram_mode) {
        // masked to fit number of ram banks
        return bank_reg_2 & (highestOrderBit(ram_banks) - 1);
    }

    return 0;
}