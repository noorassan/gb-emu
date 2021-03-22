#include "bus.h"
#include "ppu.h"
#include "interrupt.h"

#include <iostream>


PPU::PPU(GameboyDriver *driver) {
    this->driver = driver;
    pixel_line.reserve(SCREEN_WIDTH);
}

void PPU::reset() {
    for (auto &i : oam) i = 0x00;
    for (auto &i : vram) i = 0x00;
    cycles = 0;

    transfer_cycles = 0;
    pixel_line.clear();

    setStatus(OAM_SEARCH);
    write(LY, 0);
}

void PPU::clock(uint8_t clocks) {
    cycles += clocks;

    switch (getStatus()) {
        case H_BLANK: {
            uint32_t hblank_cycles = 376 - transfer_cycles;
            if (cycles >= hblank_cycles) {
                uint8_t line = read(LY);
                if (line == SCREEN_HEIGHT - 1) {
                    setStatus(V_BLANK);
                    bus->requestInterrupt(INTERRUPT::V_BLANK);
                } else {
                    setStatus(OAM_SEARCH);
                }

                write(LY, line + 1);
                cycles -= hblank_cycles;
            }
            break;
        }

        case V_BLANK: {
            if (cycles >= 456) {
                uint8_t line = read(LY);
                if (line == 153) {
                    this->driver->render();
                    setStatus(OAM_SEARCH);
                    write(LY, 0);
                } else {
                    write(LY, line + 1);
                }

                cycles -= 456;
            }
            break;
        }

        case OAM_SEARCH: {
            if (cycles >= 80) {
                fetchLine();
                setStatus(PIXEL_TRANSFER);
                cycles -= 80;
            }
            break;
        }
        
        case PIXEL_TRANSFER: {
            if (cycles >= transfer_cycles) {
                drawLine();
                setStatus(H_BLANK);
                cycles -= transfer_cycles;
            }
            break;
        }
    }
}

void PPU::fetchLine() {
    // TODO: More accurate transfer cycle emulation
    transfer_cycles = 172;

    uint8_t line = read(LY);
    uint8_t win_x = read(WX) - 7;
    uint8_t win_y = read(WY);

    // Fetch more or less BG pixels based on window
    if (isWinEnabled() && (line >= win_y) && (win_x <= SCREEN_WIDTH)) {
        fetchBG(line, win_x);
        //fetchWin();
    } else {
        fetchBG(line, SCREEN_WIDTH);
    }

    //fetchOBJ();
}

void PPU::fetchBG(uint8_t line, uint8_t num_pixels) {
    // If BG is not enabled we give unlit pixels
    if (!isBGEnabled()) {
        Pixel pixel;
        pixel.data = 0;
        pixel.unlit = 1;

        while(pixel_line.size() < num_pixels) {
            pixel_line.push_back(pixel);
        }

        return;
    }

    std::array<Pixel, 8> fetched;
    uint8_t scroll_x = read(SCX);
    uint8_t scroll_y = read(SCY);

    uint8_t skip_pixels = scroll_x % 8;

    // X & Y coordinates of our starting tile in the BG tile map
    uint8_t tile_x = (scroll_x / 8) & 0x1F;
    uint8_t tile_y = ((scroll_y + line) / 8) & 0x1F;

    // Which line of the tiles we're fetching
    uint8_t tile_line = (scroll_y + line) % 8;
    uint16_t tile_addr = getBGTilemapStart() + tile_x + tile_y * 0x20;

    uint8_t left_to_fetch = num_pixels - pixel_line.size();
    while (left_to_fetch > 0) {
        uint8_t tile_id = read(tile_addr);
        fetchTileLine(tile_id, tile_line, fetched);

        // Make sure we don't insert too many pixels
        uint8_t num_insert = std::min((uint8_t) fetched.size(), left_to_fetch);

        // Insert pixels
        pixel_line.insert(pixel_line.end(), std::begin(fetched) + skip_pixels, std::begin(fetched) + num_insert);
        skip_pixels = 0;

        tile_addr++;
        left_to_fetch = pixel_line.size() - num_pixels;
    }
}

void PPU::fetchWin(uint8_t line) {
}

void PPU::fetchTileLine(uint8_t tile_id, uint8_t tile_line, std::array<Pixel, 8> &out) {
    uint16_t addr;
    if (read(LCDC) & 0x10) {
        addr = 0x8000 + (tile_id * 0x10);
    } else {
        addr = 0x9000 + (((int8_t) tile_id) * 0x10);
    }

    addr += tile_line * 2;
    uint8_t low_color = read(addr);
    uint8_t high_color = read(addr + 1);

    for (uint8_t i = 0; i < 8; i++) {
        Pixel temp;
        temp.data = 0;
        temp.bgp = 1;
        temp.unlit = 0;
        temp.color = ((low_color & 0x01) << 0) |
                     ((high_color & 0x01) << 1);

        low_color >>= 1;
        high_color >>= 1;

        out[7 - i] = temp;
    }
}

void PPU::drawLine() {
    if (!isPPUEnabled()) {
        return;
    }

    uint8_t line = read(LY);

    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        Pixel pixel = pixel_line[x];
        uint16_t palette_addr;
        if (pixel.bgp) {
            palette_addr = BGP;
        } else if (pixel.obp0) {
            palette_addr = OBP0;
        } else if (pixel.obp1) {
            palette_addr = OBP1;
        }

        COLOR color;
        if (pixel.unlit) {
            color = UNLIT;
        } else {
            color = COLOR((read(palette_addr) >> (pixel.color * 2)) & 0x03);
        }

        this->driver->draw(color, x, line);
    }

    pixel_line.clear();
}

void PPU::statInterrupt() {
    if (((read(STAT) & 0x08) && (getStatus() == H_BLANK))    ||
        ((read(STAT) & 0x10) && (getStatus() == V_BLANK))    ||
        ((read(STAT) & 0x20) && (getStatus() == OAM_SEARCH)) ||
        ((read(STAT) & 0x40) && (read(LYC) == read(LY)))) {
        bus->requestInterrupt(LCD_STAT);
    }
}

uint16_t PPU::getBGTilemapStart() {
    if (read(LCDC) & 0x08) {
        return 0x9C00;
    } else {
        return 0x9800;
    }
}

uint16_t PPU::getWinTilemapStart() {
    if (read(LCDC) & 0x40) {
        return 0x9C00;
    } else {
        return 0x9800;
    }
}

PPU::PPU_STATUS PPU::getStatus() {
    return (PPU_STATUS) (read(STAT) & 0x03);
}

void PPU::setStatus(PPU_STATUS status) {
    uint8_t stat_val = (read(STAT) & 0xFC) | status;
    write(STAT, stat_val);
}

bool PPU::isPPUEnabled() {
    return read(LCDC) & 0x80;
}

bool PPU::isWinEnabled() {
    return (read(LCDC) & 0x21) == 0x21;
}

bool PPU::isBGEnabled() {
    return read(LCDC) & 0x01;
}

bool PPU::isOBJEnabled() {
    return read(LCDC) & 0x02;
}

uint8_t PPU::cpuRead(uint16_t addr) {
    if (addr >= 0x8000 && addr < 0xA000) {
        PPU_STATUS status = getStatus();
        if (!isPPUEnabled() || status != PIXEL_TRANSFER) {
            return vram[addr & 0x7FFF];
        }
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        PPU_STATUS status = getStatus();
        if (!isPPUEnabled() || (status != OAM_SEARCH && status != PIXEL_TRANSFER)) {
            return oam[addr & 0x01FF];
        }
    } else if (addr == LY) {
        return read(LY);
    }

    return 0xFF;
}

void PPU::cpuWrite(uint16_t addr, uint8_t data) {
    if (addr >= 0x8000 && addr < 0xA000) {
        PPU_STATUS status = getStatus();
        if (!isPPUEnabled() || status != PIXEL_TRANSFER) {
            vram[addr & 0x7FFF] = data;
        }
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        PPU_STATUS status = getStatus();
        if (!isPPUEnabled() || (status != OAM_SEARCH && status != PIXEL_TRANSFER)) {
            oam[addr & 0x01FF] = data;
        }
    } else if (addr == LY) {
        // all writes to LY reset the register
        write(LY, 0);
    }
}

bool PPU::handlesAddr(uint16_t addr) {
    return (addr == LY);
}

uint8_t PPU::read(uint16_t addr) {
    if (addr >= 0x8000 && addr < 0xA000) {
        return vram[addr & 0x7FFF];
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        return oam[addr & 0x01FF];
    } else {
        return bus->ppuRead(addr);
    }
}

void PPU::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x8000 && addr < 0xA000) {
        vram[addr & 0x7FFF] = data;
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        oam[addr & 0x00FF] = data;
     } else {
        bus->ppuWrite(addr, data);
     }
}

void PPU::connectBus(Bus *bus) {
    this->bus = bus;
}