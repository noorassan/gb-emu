#include <fstream>
#include <vector>
#include <cstdint>
#include <memory>

#include "cartridge.h"
#include "memory_bank_controllers/no_mbc.h"

#define KB 1024


Cartridge::Cartridge(const std::string &filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ifstream::binary);

    if (ifs.good()) {
        // read first 0x200 bytes into ROM bank #0
        rom.resize(0x200);
        ifs.read((char *) rom.data(), 0x200);
        
        // grab game title in a mildly sneaky manner
        char *title_ptr = (char *) &rom[0x0134];
        title = std::string(title_ptr);

        // get number of rom/ram banks based on size read from cart
        setROMSize();
        setRAMSize();

        // set MBC based on type read from cart
        setMBC();

        // read in the rest of the ROM
        ifs.read((char *) rom.data() + 0x200, rom.size() - 0x200);
    } else {
        throw std::invalid_argument("Cartridge could not be read.");
    }
}

uint8_t Cartridge::read(uint16_t addr) {
    uint16_t mapped_address;

    if (mbc->read(addr, mapped_address)) {
        return rom[mapped_address];
    }

    return 0;
}

void Cartridge::write(uint16_t addr, uint8_t data) {
    uint16_t mapped_address;

    if (mbc->write(addr, data, mapped_address)) {
        rom[mapped_address] = data;
    }
}

void Cartridge::setROMSize() {
    // not the actual size in kB
    uint8_t rom_size = rom[0x0148];
    uint8_t num_banks;
    if (rom_size <= 6) {
        num_banks = 2 << (rom_size + 1);
    } else if (rom_size == 0x52) {
        num_banks = 72;
    } else if (rom_size == 0x53) {
        num_banks = 80;
    } else if (rom_size == 0x54) {
        num_banks = 96;
    } else {
        throw std::invalid_argument("Invalid ROM size value read from cartridge.");
    }

    rom.resize(num_banks * 16 * KB);
    return;
}

void Cartridge::setRAMSize() {
    // not the actual size in kB
    uint8_t ram_size = rom[0x0149];
    uint8_t num_banks;
    if (ram_size == 0) {
        num_banks = 0;
    } else if (ram_size <= 2) {
        num_banks = 1;
    } else if (ram_size == 3) {
        num_banks = 4;
    } else if (ram_size == 4) {
        num_banks = 16;
    } else {
        throw std::invalid_argument("Invalid RAM size value read from cartridge.");
    }

    ram.resize(num_banks * 2 * KB);
    return;
}

void Cartridge::setMBC() {
    uint8_t mbc_type = rom[0x0147];

    if (mbc_type == 0) {
        mbc = std::make_shared<NoMBC>();
    } else {
        throw std::invalid_argument("Invalid Memory Bank Controller type read from cartridge.");
    }
    // TODO: add other memory bank controllers
    return;
}