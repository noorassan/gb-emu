#include <fstream>
#include <vector>
#include <cstdint>

#include "cartridge.h"
#include "mbc/mbc_1.h"
#include "mbc/mbc_3.h"
#include "mbc/mbc_5.h"
#include "mbc/no_mbc.h"


Cartridge::Cartridge(const std::string &filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ifstream::binary);

    if (ifs.good()) {
        // read first 0x200 bytes into ROM bank #0
        rom.resize(0x4000);
        ifs.read((char *) rom.data(), 0x200);
        
        // grab game title in a mildly sneaky manner
        char *title_ptr = (char *) &rom[0x0134];
        title = std::string(title_ptr, 0x0F);

        // get number of rom/ram banks based on size read from cart
        setROMSize();
        setRAMSize();

        // set MBC based on type read from cart
        setMBC();

        // read in the rest of the ROM
        ifs.read(((char *) rom.data()) + 0x200, rom.size() - 0x200);
    } else {
        throw std::invalid_argument("Cartridge could not be read.");
    }
}

uint8_t Cartridge::read(uint16_t addr) {
    uint32_t mapped_address;
    uint8_t data = 0x00;

    if (mbc->read(addr, mapped_address, data)) {
        if (addr <= 0x8000) {
            if (mapped_address >= rom.size()) {
                throw std::runtime_error("Attempted to read past end of ROM");
            }

            return rom[mapped_address];
        } else {
            if (mapped_address >= ram.size()) {
                throw std::runtime_error("Attempted to read past end of RAM");
            }

            return ram[mapped_address];
        }
    }

    return data;
}

void Cartridge::write(uint16_t addr, uint8_t data) {
    uint32_t mapped_address;

    // We need to call mbc->write because this might be a write to ROM
    // that alters the MBC state. If the MBC tells us that this is a write
    // we actually need to perform then we do so.
    if (mbc->write(addr, data, mapped_address)) {
        if (mapped_address >= ram.size()) {
            throw std::runtime_error("Attempted to write past end of RAM");
        }

        ram[mapped_address] = data;
    }
}

// Altering the returned vector will affect cartridge state so use cautiously
std::vector<uint8_t> &Cartridge::getRAM() {
    return ram;
}

std::string Cartridge::getTitle() {
    return title;
}

void Cartridge::setROMSize() {
    // not the actual size in kB
    uint8_t rom_size = rom[0x0148];

    if (rom_size <= 8) {
        rom_banks = 2 << rom_size;
    } else {
        throw std::invalid_argument("Invalid ROM size value read from cartridge.");
    }

    rom.resize(rom_banks * 16 * KB);
    return;
}

void Cartridge::setRAMSize() {
    // not the actual size in kB
    uint8_t ram_size = rom[0x0149];

    if (ram_size == 0) {
        ram_banks = 0;
    } else if (ram_size == 2) {
        ram_banks = 1;
    } else if (ram_size == 3) {
        ram_banks = 4;
    } else if (ram_size == 4) {
        ram_banks = 16;
    } else if (ram_size == 5) {
        ram_banks = 8;
    } else {
        throw std::invalid_argument("Invalid RAM size value read from cartridge.");
    }

    ram.resize(ram_banks * 8 * KB);
    for (auto &i : ram) i = 0x00;
    return;
}

void Cartridge::setMBC() {
    uint8_t mbc_type = rom[0x0147];

    if (mbc_type == 0) {
        mbc = std::make_shared<NoMBC>(rom_banks, ram_banks);
    } else if (mbc_type >= 0x01 && mbc_type <= 0x03){
        mbc = std::make_shared<MBC1>(rom_banks, ram_banks);
    } else if (mbc_type >= 0x0F && mbc_type <= 0x13) {
       mbc = std::make_shared<MBC3>(rom_banks, ram_banks);
    } else if (mbc_type >= 0x19 && mbc_type <= 0x1E) {
        mbc = std::make_shared<MBC5>(rom_banks, ram_banks);
    } else {
        throw std::invalid_argument("Invalid Memory Bank Controller type read from cartridge.");
    }
    // TODO: add other memory bank controllers
    return;
}