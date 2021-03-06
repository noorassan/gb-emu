#include <iostream>

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
    } else if (addr >= 0xFF00 && addr <= 0xFFFF) {
        // Writes in this address range are delegated to devices

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
        if (handleDMA(addr, data)) {
            return;
        }

        zero_page_ram[addr & 0x00FF] = data;
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
        // Reads in this address range are delegated to devices

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

        return zero_page_ram[addr & 0x00FF];
    }

    return 0;
}

void Bus::requestInterrupt(INTERRUPT intr) {
    cpu.requestInterrupt(intr);
}

void Bus::reset() {
    for (auto &i : ram) i = 0x00;
    for (auto &i : zero_page_ram) i = 0x00;

    // reset memory locations to default values
    zero_page_ram[0x05] = 0x00; zero_page_ram[0x06] = 0x00; zero_page_ram[0x07] = 0x00; zero_page_ram[0x10] = 0x80;
    zero_page_ram[0x11] = 0xBF; zero_page_ram[0x12] = 0xF3; zero_page_ram[0x14] = 0xBF; zero_page_ram[0x16] = 0x3F;
    zero_page_ram[0x17] = 0x00; zero_page_ram[0x19] = 0xBF; zero_page_ram[0x1A] = 0x7F; zero_page_ram[0x1B] = 0xFF;
    zero_page_ram[0x1C] = 0x9F; zero_page_ram[0x1E] = 0xBF; zero_page_ram[0x20] = 0xFF; zero_page_ram[0x21] = 0x00;
    zero_page_ram[0x22] = 0x00; zero_page_ram[0x23] = 0xBF; zero_page_ram[0x24] = 0x77; zero_page_ram[0x25] = 0xF3;
    zero_page_ram[0x26] = 0xF1; zero_page_ram[0x40] = 0x91; zero_page_ram[0x42] = 0x00; zero_page_ram[0x43] = 0x00;
    zero_page_ram[0x45] = 0x00; zero_page_ram[0x47] = 0xFC; zero_page_ram[0x48] = 0xFF; zero_page_ram[0x49] = 0xFF;
    zero_page_ram[0x4A] = 0x00; zero_page_ram[0x4B] = 0x00; zero_page_ram[0xFF] = 0x00;

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
    std::vector<uint8_t> &ram = this->cart->getRAM();

    // if the cartridge has no ram, we return
    if (!ram.size()) {
        return;
    }

    std::ofstream ofs(filename);

    ofs.write((char *) ram.data(), ram.size());
    ofs.close();
}

void Bus::loadState(const std::string &filename) {
    std::ifstream ifs(filename);

    // if the file doesn't exist, we return
    if (!ifs.good()) {
        return;
    }

    std::vector<uint8_t> &ram = this->cart->getRAM();
    ifs.read((char *) ram.data(), ram.size());
    ifs.close();
}

bool Bus::handleDMA(uint16_t addr, uint8_t data) {
    if (addr != 0xFF46) {
        return false;
    }

    uint16_t dma_addr = data << 8;
    for (uint8_t i = 0; i < OAM_SIZE; i++) {
        cpuWrite(OAM_START + i, cpuRead(dma_addr + i));
    }

    return true;
}