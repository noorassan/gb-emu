#include <iostream>

#include "bus.h"


Bus::Bus(GameboyDriver *driver) : ppu(driver) {
    this->driver = driver;

    ppu.connectBus(this);
    cpu.connectBus(this);
    timer.connectBus(this);
}

void Bus::cpuWrite(uint16_t addr, uint8_t data) {
    if (addr == 0xFF01) {
        std::cout << data;
    }

    if (addr >= 0x0000 && addr < 0x8000) {
        cart->write(addr, data);
    } else if (addr >= 0x8000 && addr < 0xA000) {
        ppu.cpuWrite(addr, data);
    } else if (addr >= 0xA000 && addr < 0xC000) {
        cart->write(addr, data);
    } else if (addr >= 0xC000 && addr < 0xFE00) {
        ram[addr & 0x1FFF] = data;
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        ppu.cpuWrite(addr, data);
    } else if (addr >= 0xFF00 && addr <= 0xFFFF) {
        if (timer.handlesAddr(addr)) {
            timer.cpuWrite(addr, data);
        } else if(ppu.handlesAddr(addr)) {
            ppu.cpuWrite(addr, data);
        } else {
            zero_page_ram[addr & 0x00FF] = data;
        }
    } 
}

uint8_t Bus::cpuRead(uint16_t addr) {
    if (addr >= 0x0000 && addr < 0x8000) {
        return cart->read(addr);
    } else if (addr >= 0x8000 && addr < 0xA000) {
        return ppu.cpuRead(addr);
    } else if (addr >= 0xA000 && addr < 0xC000) {
        return cart->read(addr);
    } else if (addr >= 0xC000 && addr < 0xFE00) {
        return ram[addr & 0x1FFF];
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        return ppu.cpuRead(addr);
    } else if (addr >= 0xFF00 && addr <= 0xFFFF) {
        if (timer.handlesAddr(addr)) {
            return timer.cpuRead(addr);
        } else if (ppu.handlesAddr(addr)) {
            return ppu.cpuRead(addr);
        } else {
            return zero_page_ram[addr & 0x00FF];
        }
    }

    return 0;
}

void Bus::timerWrite(uint16_t addr, uint8_t data) {
    zero_page_ram[addr & 0x00FF] = data;
}

uint8_t Bus::timerRead(uint16_t addr) {
    return zero_page_ram[addr & 0x00FF];
}

void Bus::ppuWrite(uint16_t addr, uint8_t data) {
    zero_page_ram[addr & 0x00FF] = data;
}

uint8_t Bus::ppuRead(uint16_t addr) {
    return zero_page_ram[addr & 0x00FF];
}

void Bus::requestInterrupt(INTERRUPT intr) {
    cpuWrite(IF, cpuRead(IF) | intr);
}

void Bus::reset() {
    for (auto &i : ram) i = 0x00;
    for (auto &i : zero_page_ram) i = 0x00;

    // reset memory locations to default values
    cpuWrite(0xFF05, 0x00); cpuWrite(0xFF06, 0x00); cpuWrite(0xFF07, 0x00); cpuWrite(0xFF10, 0x80);
    cpuWrite(0xFF11, 0xBF); cpuWrite(0xFF12, 0xF3); cpuWrite(0xFF14, 0xBF); cpuWrite(0xFF16, 0x3F);
    cpuWrite(0xFF17, 0x00); cpuWrite(0xFF19, 0xBF); cpuWrite(0xFF1A, 0x7F); cpuWrite(0xFF1B, 0xFF);
    cpuWrite(0xFF1C, 0x9F); cpuWrite(0xFF1E, 0xBF); cpuWrite(0xFF20, 0xFF); cpuWrite(0xFF21, 0x00);
    cpuWrite(0xFF22, 0x00); cpuWrite(0xFF23, 0xBF); cpuWrite(0xFF24, 0x77); cpuWrite(0xFF25, 0xF3);
    cpuWrite(0xFF26, 0xF1); cpuWrite(0xFF40, 0x91); cpuWrite(0xFF42, 0x00); cpuWrite(0xFF43, 0x00);
    cpuWrite(0xFF45, 0x00); cpuWrite(0xFF47, 0xFC); cpuWrite(0xFF48, 0xFF); cpuWrite(0xFF49, 0xFF);
    cpuWrite(0xFF4A, 0x00); cpuWrite(0xFF4B, 0x00); cpuWrite(0xFFFF, 0x00);

    cpu.reset();
    ppu.reset();
}

void Bus::run() {
    bool quit = false;
    uint32_t cycles;

    while(!quit) {
        cycles = 0;
        while (cycles <= POLL_INTERVAL) {
            uint8_t elapsed = cpu.clock();
            ppu.clock(elapsed);
            timer.clock(elapsed);

            cycles += elapsed;
        }

        // Handle control input
        CONTROL control = this->driver->pollControls();
        quit = (control == QUIT);
    }
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge> cart) {
    this->cart = cart;
}
