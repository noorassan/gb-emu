#include "bus.h"
#include <iostream>


Bus::Bus() {
    cpu.connectBus(this);
    timer.connectBus(this);
    
    // zero out ram
    for (uint8_t &i : ram) i = 0x00;
}

void Bus::write(uint16_t addr, uint8_t data) {
    // Write serial i/o data to console
    if (addr == 0xFF01) {
        std::cout << data;
    }
    
    if (addr >= 0x0000 && addr < 0x8000) {
        cart->write(addr, data);
    } else if (addr >= 0x8000 && addr < 0xA000) {
        ppu.write(addr, data);
    } else if (addr >= 0xA000 && addr < 0xC000) {
        cart->write(addr, data);
    } else if (addr >= 0xC000 && addr < 0xFE00) {
        ram[addr & 0x1FFF] = data;
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        ppu.write(addr, data);
    } else if (addr >= 0xFF00 && addr < 0xFF4C) {
        io_ports[addr & 0x00FF] = data;
    } else if (addr >= 0xFF80 && addr <= 0xFFFF) {
        zero_page_ram[addr] = data;
    }
}

uint8_t Bus::read(uint16_t addr) {
    if (addr >= 0x0000 && addr < 0x8000) {
        return cart->read(addr);
    } else if (addr >= 0x8000 && addr < 0xA000) {
        return ppu.read(addr);
    } else if (addr >= 0xA000 && addr < 0xC000) {
        return cart->read(addr);
    } else if (addr >= 0xC000 && addr < 0xFE00) {
        return ram[addr & 0x1FFF];
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        return ppu.read(addr);
    } else if (addr >= 0xFF00 && addr < 0xFF4C) {
        return io_ports[addr & 0x00FF];
    } else if (addr >= 0xFF80 && addr <= 0xFFFF) {
        return zero_page_ram[addr];
    }

    return 0;
}

void Bus::requestInterrupt(INTERRUPT intr) {
    write(IF, read(IF) | intr);
}

void Bus::reset() {
    cpu.reset();

    // zero out ram
    for (uint8_t &i : ram) i = 0x00;

    // reset memory values to default values
    ram[0xFF05] = 0x00; ram[0xFF06] = 0x00; ram[0xFF07] = 0x00; ram[0xFF10] = 0x80;
    ram[0xFF11] = 0xBF; ram[0xFF12] = 0xF3; ram[0xFF14] = 0xBF; ram[0xFF16] = 0x3F;
    ram[0xFF17] = 0x00; ram[0xFF19] = 0xBF; ram[0xFF1A] = 0x7F; ram[0xFF1B] = 0xFF;
    ram[0xFF1C] = 0x9F; ram[0xFF1E] = 0xBF; ram[0xFF20] = 0xFF; ram[0xFF21] = 0x00;
    ram[0xFF22] = 0x00; ram[0xFF23] = 0xBF; ram[0xFF24] = 0x77; ram[0xFF25] = 0xF3;
    ram[0xFF26] = 0xF1; ram[0xFF40] = 0x91; ram[0xFF42] = 0x00; ram[0xFF43] = 0x00;
    ram[0xFF45] = 0x00; ram[0xFF47] = 0xFC; ram[0xFF48] = 0xFF; ram[0xFF49] = 0xFF; 
    ram[0xFF4A] = 0x00; ram[0xFF4B] = 0x00; ram[0xFFFF] = 0x00;
}

void Bus::clock() {
    timer.tick();

    if (clock_cycles % 2 == 0) {
        ppu.clock();
    }

    if (clock_cycles % 4 == 0) {
        cpu.clock();
    }
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge> cart) {
    this->cart = cart;
}
