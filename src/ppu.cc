#include "bus.h"
#include "ppu.h"

uint8_t PPU::read(uint16_t addr) {
    if (addr >= 0x8000 && addr < 0xA000) {
        return vram[addr && 0x7FFF];
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        return oam[addr & 0x01FF];
    }

    return 0;
}

void PPU::write(uint16_t addr, uint8_t data) {}