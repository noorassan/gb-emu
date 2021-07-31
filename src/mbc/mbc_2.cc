#include <fstream>

#include "bit_utils.h"
#include "mbc/mbc_2.h"

MBC2::MBC2(uint16_t rom_banks, uint8_t ram_banks) : MBC(rom_banks, ram_banks) {
    ram_enabled = false;
    rom_bank = 0x01;
    
    for (auto &i : ram) i = 0x00;
}

bool MBC2::read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) {
    if (addr >= 0x0000 && addr < 0x4000) {
        mapped_addr = addr;
        return true;
    } else if (addr >= 0x4000 && addr < 0x8000) {
        mapped_addr = mapROMAddress(addr, rom_bank);
        return true;    
    } else if (addr >= 0xA000 && addr < 0xC000) {
        if (ram_enabled) {
            data = ram[data & 0x01FF];
            return false;
        }
    }

    data = 0xFF;
    return false;
}

bool MBC2::write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) {
    (void) mapped_addr;

    if (addr >= 0x0000 && addr < 0x4000) {
        if (addr & 0x0100) {
            rom_bank = data & 0x0F & (highestOrderBit(rom_banks) - 1);
            if (!rom_bank) rom_bank = 1;
        } else {
            ram_enabled = data == 0x0A;
        }
    } else if (addr >= 0xA000 && addr < 0xC000) {
        if (ram_enabled) {
            ram[addr & 0x01FF] = data;
        }
    }

    return false;
}

void MBC2::saveRAM(std::ofstream &ofs) {
    ofs.write((char *) ram.data(), ram.size());
}

void MBC2::loadRAM(std::ifstream &ifs) {
    ifs.read((char *) ram.data(), ram.size());
}