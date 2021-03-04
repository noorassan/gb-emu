#include "bus.h"
#include "ppu.h"
#include "interrupt.h"

#include <iostream>


PPU::PPU(DRAW) {
    this->draw = draw;
}

void PPU::reset() {
    for (auto &i : oam) i = 0x00;
    for (auto &i : vram) i = 0x00;

    setStatus(OAM_SEARCH);
    new_status = true;

    pixel_fifo.clear();
    pending_fifo.clear();

    cycles = 0;
    setLY(0);
}

void PPU::clock() {
    switch(getStatus()) {
        case H_BLANK:
            if (new_status) {
                cycles += 188 - fetch_cycles;
                new_status = false;
            } else if (cycles == 1) {
                if (getLY() == 143) {
                    setStatus(V_BLANK);
                } else {
                    setStatus(OAM_SEARCH);
                }
                new_status = true;
                setLY(getLY() + 1);
            }
            break;

        case V_BLANK:
            if (new_status) {
                cycles += 228;
                new_status = false;
            } else if (cycles == 1) {
                if (getLY() == 153) {
                    setStatus(OAM_SEARCH);
                    new_status = true;
                    setLY(0);
                } else {
                    setLY(getLY() + 1);
                    cycles += 228;
                }
            }
            break;

        case OAM_SEARCH:
            if (new_status) {
                cycles += 40;
                new_status = false;
            } else if (cycles == 1) {
                setStatus(PIXEL_TRANSFER);
                new_status = true;
            }
            break;
        
        case PIXEL_TRANSFER:
            if (new_status) {
                x_pos = 0;
                tile_num = 0;
                skip_pixels = ppuRead(SCX) % 8;
                fetch_cycles = 0;

                pixel_fifo.clear();
                pending_fifo.clear();
                new_status = false;
            }

            if (cycles == 0) {
                uint8_t elapsed = fetchPixels();
                cycles += elapsed;
                fetch_cycles += elapsed;
            }

            clockPixelFIFO();
            clockPixelFIFO();
    
            if (x_pos == SCREEN_WIDTH) {
                new_status = true;
                setStatus(H_BLANK);
                cycles = 1;
            }
            break;
    }

    cycles--;
}

void PPU::clockPixelFIFO() {
    if (skip_pixels > 0 && pixel_fifo.size() > 0) {
        pixel_fifo.erase(pixel_fifo.begin());
        skip_pixels--;
    } else if (skip_pixels == 0 && pixel_fifo.size() > 8) {
        Pixel pixel = pixel_fifo[0];
        pixel_fifo.erase(pixel_fifo.begin());

        uint16_t palette_addr;
        if (pixel.bgp) {
            palette_addr = BGP;
        } else if (pixel.obp0) {
            palette_addr = OBP0;
        } else if (pixel.obp1) {
            palette_addr = OBP1;
        } else {
            throw std::invalid_argument("Cannot output pixel without palette.");
        }

        COLOR color = COLOR((ppuRead(palette_addr) >> (pixel.color * 2)) & 0x03);
        (*draw)(color, x_pos, getLY());
        x_pos++;
    }
}

uint8_t PPU::fetchPixels() {
    if (pending_fifo.size() == 8) {
        if (pixel_fifo.size() <= 8) {
            pixel_fifo.insert(pixel_fifo.end(),
                              pending_fifo.begin(),
                              pending_fifo.end());

            pending_fifo.clear();
        }

        return 1;
    } else {
        uint16_t tilemap_addr = getBGTilemapStart();

        uint8_t tile_x = ((ppuRead(SCX) / 8) + tile_num) & 0x1F;
        uint8_t tile_y = ((ppuRead(SCY) + getLY()) / 8) & 0x1F;
        uint8_t tile_id = ppuRead(tilemap_addr + (tile_y * 0x20) + tile_x);
        
        fetchTileLine(tile_id);
        
        tile_num++;
        return 3;
    }

}

void PPU::fetchTileLine(uint8_t tile_id) {
    uint16_t addr;
    if (ppuRead(LCDC) & 0x10) {
        addr = 0x8000 + (tile_id * 0x10);
    } else {
        addr = 0x8800 + (((int8_t) tile_id) * 0x10);
    }

    addr += ((ppuRead(SCY) + getLY()) % 8) * 2;
    uint8_t low_color = ppuRead(addr);
    uint8_t high_color = ppuRead(addr + 1);
    for (uint8_t i = 0; i < 8; i++) {
        Pixel temp;
        temp.data = 0;
        temp.color = (((low_color >> i) & 0x01) << 0) | 
                     (((high_color >> i) & 0x01) << 1);
        temp.bgp = 1;
        pending_fifo.insert(pending_fifo.begin(), temp);
    }
}

uint16_t PPU::getBGTilemapStart() {
    if (ppuRead(LCDC) & 0x08) {
        return 0x9C00;
    } else {
        return 0x9800;
    }
}

uint16_t PPU::getWinTilemapStart() {
    if (ppuRead(LCDC) & 0x40) {
        return 0x9C00;
    } else {
        return 0x9800;
    }
}

PPU::PPU_STATUS PPU::getStatus() {
    return PPU_STATUS(ppuRead(STAT) & 0x03);
}

void PPU::setStatus(PPU_STATUS status) {
    uint8_t stat_val = (ppuRead(STAT) & 0xFC) | status;
    ppuWrite(STAT, stat_val);
}

uint8_t PPU::getLY() {
    return ppuRead(LY);
}

void PPU::setLY(uint8_t val) {
    ppuWrite(LY, val);
}

uint8_t PPU::busRead(uint16_t addr) {
    if (addr >= 0x8000 && addr < 0xA000) {
        PPU_STATUS status = getStatus();
        if (status != PIXEL_TRANSFER) {
            return vram[addr & 0x7FFF];
        }
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        PPU_STATUS status = getStatus();
        if (status != OAM_SEARCH && status != PIXEL_TRANSFER) {
            return oam[addr & 0x01FF];
        }
    }

    return 0xFF;
}

void PPU::busWrite(uint16_t addr, uint8_t data) {
    if (addr >= 0x8000 && addr < 0xA000) {
        PPU_STATUS status = getStatus();
        if (status != PIXEL_TRANSFER) {
            vram[addr & 0x7FFF] = data;
        }
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        PPU_STATUS status = getStatus();
        if (status != OAM_SEARCH && status != PIXEL_TRANSFER) {
            oam[addr & 0x01FF] = data;
        }
    }
}

uint8_t PPU::ppuRead(uint16_t addr) {
    if (addr >= 0x8000 && addr < 0xA000) {
        return vram[addr & 0x7FFF];
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        return oam[addr & 0x01FF];
    } else {
        return bus->read(addr);
    }
}

void PPU::ppuWrite(uint16_t addr, uint8_t data) {
    if (addr >= 0x8000 && addr < 0xA000) {
        vram[addr & 0x7FFF] = data;
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        oam[addr & 0x00FF] = data;
     } else {
        bus->write(addr, data);
     }
}

void PPU::connectBus(Bus *bus) {
    this->bus = bus;
}