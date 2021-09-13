#include "bus.h"

Bus::Bus(GameboyDriver *driver) : ppu(driver), apu(driver), controls(driver) {
    this->driver = driver;

    cpu.connectBus(this);
    ppu.connectBus(this);
    timer.connectBus(this);
    controls.connectBus(this);

    reset();
}

void Bus::cpuWrite(uint16_t addr, uint8_t data) {
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
    } else if (addr >= 0xFF00 && addr < 0xFF4C) {
        handleIOWrite(addr, data);
    } else if (addr >= 0xFF80) {
        high_ram[addr & 0x7F] = data;
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
    } else if (addr >= 0xFF00 && addr < 0xFF4C) {
        return handleIORead(addr);
    } else if (addr >= 0xFF80) {
        return high_ram[addr & 0x7F];
    }

    return 0x00;
}

void Bus::requestInterrupt(INTERRUPT intr) {
    cpu.requestInterrupt(intr);
}

void Bus::reset() {
    for (auto &i : ram) i = 0x00;
    for (auto &i : high_ram) i = 0x00;

    cpu.reset();
    ppu.reset();
    apu.reset();
}

void Bus::run() {
    uint32_t cycles;

    while(!driver->quitReceived()) {
        // Poll controls for a quit
        controls.updateControls();

        cycles = 0;
        while (cycles <= POLL_INTERVAL) {
            uint8_t elapsed = cpu.clock();
            apu.clock(elapsed);
            ppu.clock(elapsed);
            timer.clock(elapsed);

            cycles += elapsed;
        }
    }
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge> cart) {
    this->cart = cart;
}

void Bus::saveState(const std::string &filename) {
    std::ofstream ofs(filename);
    cart->saveRAM(ofs);
    ofs.close();
}

void Bus::loadState(const std::string &filename) {
    std::ifstream ifs(filename);
    if (ifs.good()) cart->loadRAM(ifs);
    ifs.close();
}

void Bus::handleDMA(uint8_t data) {
    uint16_t dma_addr = data << 8;
    for (uint8_t i = 0; i < OAM_SIZE; i++) {
        cpuWrite(OAM_START + i, cpuRead(dma_addr + i));
    }
}

void Bus::handleIOWrite(uint16_t addr, uint8_t data) {
    if (timer.regWrite(addr, data)) {
        return;
    }
    if(ppu.regWrite(addr, data)) {
        return;
    }
    if (controls.regWrite(addr, data)) {
        return;
    }
    if (apu.regWrite(addr, data)) {
        return;
    }

    switch(addr) {
        case DMA: handleDMA(data); break;
        case SB: sb = data; break;
        case SC: sc = data; break;
        case IF: intr_flag = data; break;
    }
}

uint8_t Bus::handleIORead(uint16_t addr) {
    uint8_t val;
    if (timer.regRead(addr, val)) {
        return val;
    }
    if (ppu.regRead(addr, val)) {
        return val;
    }
    if (apu.regRead(addr, val)) {
        return val;
    }
    if (controls.regRead(addr, val)) {
        return val;
    }

    switch(addr) {
        case SB: return sb; break;
        case SC: return sc; break;
        case IF: return intr_flag; break;
    }

    return 0x00;
}