#pragma once

#include <cstdint>


class MBC {
    public:
        MBC(uint16_t rom_banks, uint8_t ram_banks) : rom_banks(rom_banks), ram_banks(ram_banks) {}
        virtual ~MBC() = default;
    
    public:
        // MBC processes a read to the specified address
        // If returns true, then the read should be made to ROM/RAM using the mapped address
        // If returns false, then data should be taken as the result of the read
        virtual bool read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) = 0;

        // MBC processes a write to the specified address
        // If returns true, then the write should be made to RAM using the mapped address
        // If returns false, then no action should be taken
        virtual bool write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) = 0;

    protected:
        const uint16_t rom_banks;
        const uint8_t ram_banks;

    protected:
        uint32_t mapROMAddress(uint16_t addr, uint16_t bank) {
            return (addr & 0x3FFF) + 0x4000 * bank;
        }

        uint32_t mapRAMAddress(uint16_t addr, uint8_t bank) {
            return (addr & 0x1FFF) + 0x2000 * bank;
        }
};