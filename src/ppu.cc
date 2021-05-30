#include "bus.h"
#include "interrupt.h"
#include "ppu.h"

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
    ly = 0;
}

void PPU::clock(uint8_t clocks) {
    cycles += clocks;

    switch (getStatus()) {
        case H_BLANK: {
            uint32_t hblank_cycles = 376 - transfer_cycles;
            if (cycles >= hblank_cycles) {
                if (ly == SCREEN_HEIGHT - 1) {
                    setStatus(V_BLANK);
                    bus->requestInterrupt(INTERRUPT::V_BLANK);
                } else {
                    searchOAM(ly + 1);
                    setStatus(OAM_SEARCH);
                }

                ly++;
                cycles -= hblank_cycles;
            }
            break;
        }

        case V_BLANK: {
            if (cycles >= 456) {
                if (ly == 153) {
                    this->driver->render();
                    searchOAM(ly + 1);
                    setStatus(OAM_SEARCH);
                    ly = 0;
                } else {
                    ly++;
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

    checkSTATInterrupt();
}

void PPU::searchOAM(uint8_t line) {
    sprites.clear();

    uint16_t addr = OAM_START;
    uint8_t obj_height = getOBJHeight();

    while ((sprites.size() < 10) && (addr < OAM_END)) {
        Sprite sprite;
        sprite.pos_y    = read(addr);
        sprite.pos_x    = read(addr + 1);
        sprite.tile_num = read(addr + 2);
        sprite.flags    = read(addr + 3);

        if (((sprite.pos_y - 16) <= line) && ((sprite.pos_y - 16 + obj_height) >= line)) {
            sprites.push_back(sprite);
        }

        addr += 4;
    }

}

void PPU::fetchLine() {
    pixel_line.clear();

    transfer_cycles = 172;
    
    // If PPU is not enabled, we give unlit pixels
    if (!isPPUEnabled()) {
        Pixel pixel;
        pixel.data = 0;
        pixel.unlit = 1;

        while(pixel_line.size() < SCREEN_WIDTH) {
            pixel_line.push_back(pixel);
        }

        return;
    }

    uint8_t win_x = std::max(0, wx - 7); // WX values of 0-7 act weirdly so we just set those to 0
    uint8_t win_y = wy;

    // Fetch more or less BG pixels based on window
    if (isWinEnabled() && (ly >= win_y) && (win_x <= SCREEN_WIDTH)) {
        fetchBG(ly, win_x);
        fetchWin((ly - win_y), (SCREEN_WIDTH - win_x));
    } else {
        fetchBG(ly, SCREEN_WIDTH);
    }

    fetchOBJ(ly);
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

    uint8_t skip_pixels = scx % 8;

    // X & Y coordinates of our starting tile in the BG tile map
    uint8_t tile_x = (scx / 8) & 0x1F;
    uint8_t tile_y = ((scy + line) / 8) & 0x1F;

    // Which line of the tiles we're fetching
    uint8_t tile_line = (scy + line) % 8;
    uint16_t line_start = getBGTilemapStart() + tile_y * 0x20;

    while (num_pixels > 0) {
        uint16_t tile_addr = line_start + tile_x;
        uint8_t tile_id = read(tile_addr);
        fetchTileLine(tile_id, tile_line, fetched);

        // Make sure we don't insert too many pixels
        uint8_t num_insert = std::min((uint8_t) 8, num_pixels) - skip_pixels;

        // Insert pixels
        pixel_line.insert(pixel_line.end(),
                          std::begin(fetched) + skip_pixels,
                          std::begin(fetched) + skip_pixels + num_insert);

        skip_pixels = 0;

        tile_x = (tile_x + 1) & 0x1F;
        num_pixels -= num_insert;
    }
}

void PPU::fetchWin(uint8_t win_line, uint8_t num_pixels) {
    std::array<Pixel, 8> fetched;
    uint8_t tile_x = 0;
    uint8_t tile_y = (win_line / 8) & 0x1F;

    uint8_t tile_line = win_line % 8;
    uint16_t line_start = getWinTilemapStart() + tile_y * 0x20;

    while (num_pixels > 0) {
        uint16_t tile_addr = line_start + tile_x;
        uint8_t tile_id = read(tile_addr);
        fetchTileLine(tile_id, tile_line, fetched);

        // Make sure we don't insert too many pixels
        uint8_t num_insert = std::min((uint8_t) 8, num_pixels);

        // Insert pixels
        pixel_line.insert(pixel_line.end(), std::begin(fetched), std::begin(fetched) + num_insert);

        tile_x = (tile_x + 1) % 0x1F;
        num_pixels -= num_insert;
    }
}

void PPU::fetchOBJ(uint8_t line) {
    if (!isOBJEnabled()) {
        return;
    }

    // Sprites with smaller x position will overwrite those with a larger x,
    // so we add them to the line from right to left
    std::sort(sprites.begin(), sprites.end(), std::greater());

    std::array<Pixel, 8> fetched;
    for (const auto &sprite : sprites) {
        fetchOBJLine(sprite, line, fetched);

        for (uint8_t i = 0; i < 8; i++) {
            uint8_t x = sprite.pos_x - 8 + i;

            if ((x > 0 && x < SCREEN_WIDTH) && fetched[i].color != 0 && 
                (pixel_line[x].color == 0 || (~sprite.flags & 0x80))) {
                pixel_line[x] = fetched[i];
            }
        }
    }
}

void PPU::fetchTileLine(uint8_t tile_id, uint8_t tile_line, std::array<Pixel, 8> &out) {
    uint16_t addr;
    if (lcdc & 0x10) {
        addr = 0x8000 + (tile_id * 0x10);
    } else {
        addr = 0x9000 + (((int8_t) tile_id) * 0x10);
    }

    addr += tile_line * 2;
    uint8_t low_byte = read(addr);
    uint8_t high_byte = read(addr + 1);

    decodePixels(low_byte, high_byte, out);
    for (auto &pixel : out) {
        pixel.bgp = 1;
    }
}

void PPU::fetchOBJLine(const Sprite &sprite, uint8_t curr_line, std::array<Pixel, 8> &out) {
    uint8_t obj_height = getOBJHeight();
    uint8_t adjusted_tile_num = sprite.tile_num;

    if (obj_height == 16) {
        adjusted_tile_num >>= 1;
    }

    uint16_t addr = 0x8000 + adjusted_tile_num * obj_height * 2;
    uint8_t sprite_line = (curr_line - (sprite.pos_y - 16));

    // If sprite is vertically flipped
    if (sprite.flags & 0x40) {
        sprite_line = obj_height - sprite_line;
    }

    addr += sprite_line * 2;
    uint8_t low_byte = read(addr);
    uint8_t high_byte = read(addr + 1);

    decodePixels(low_byte, high_byte, out);
    for (auto &pixel : out) {
        if (sprite.flags & 0x10) {
            pixel.obp1 = 1;
        } else {
            pixel.obp0 = 1;
        }
    }

    // If sprite is horizontally flipped
    if (sprite.flags & 0x20) {
        std::reverse(std::begin(out), std::end(out));
    }
}

void PPU::decodePixels(uint8_t low_byte, uint8_t high_byte, std::array<Pixel, 8> &out) {
    for (uint8_t i = 0; i < 8; i++) {
        Pixel temp;
        temp.data = 0;
        temp.unlit = 0;
        temp.color = ((low_byte & 0x01) << 0) |
                     ((high_byte & 0x01) << 1);

        low_byte >>= 1;
        high_byte >>= 1;

        out[7 - i] = temp;
    }
}

void PPU::drawLine() {
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        Pixel pixel = pixel_line[x];
        uint8_t palette;
        if (pixel.bgp) {
            palette = bgp;
        } else if (pixel.obp0) {
            palette = obp0;
        } else if (pixel.obp1) {
            palette = obp1;
        }

        COLOR color;
        if (pixel.unlit) {
            color = UNLIT;
        } else {
            color = COLOR((palette >> (pixel.color * 2)) & 0x03);
        }

        this->driver->draw(color, x, ly);
    }
}

void PPU::checkSTATInterrupt() {
    if (((stat & 0x08) && (getStatus() == H_BLANK))    ||
        ((stat & 0x10) && (getStatus() == V_BLANK))    ||
        ((stat & 0x20) && (getStatus() == OAM_SEARCH)) ||
        ((stat & 0x40) && (lyc == ly))) {
        bus->requestInterrupt(LCD_STAT);
    }
}

uint16_t PPU::getBGTilemapStart() {
    if (lcdc & 0x08) {
        return 0x9C00;
    } else {
        return 0x9800;
    }
}

uint16_t PPU::getWinTilemapStart() {
    if (lcdc & 0x40) {
        return 0x9C00;
    } else {
        return 0x9800;
    }
}

uint8_t PPU:: getOBJHeight() {
    if (lcdc & 0x04) {
        return 16;
    } else {
        return 8;
    }
}

PPU::PPU_STATUS PPU::getStatus() {
    return (PPU_STATUS) (stat & 0x03);
}

void PPU::setStatus(PPU_STATUS status) {
    uint8_t stat_val = (stat & 0xFC) | status;
    stat = stat_val;
}

bool PPU::isPPUEnabled() {
    return lcdc & 0x80;
}

bool PPU::isWinEnabled() {
    return (lcdc & 0x21) == 0x21;
}

bool PPU::isBGEnabled() {
    return lcdc & 0x01;
}

bool PPU::isOBJEnabled() {
    return lcdc & 0x02;
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
    }
}

bool PPU::regRead(uint16_t addr, uint8_t &val) {
    switch(addr) {
        case LCDC: val = lcdc; break;
        case STAT: val = stat; break;
        case SCY:  val = scy;  break;
        case SCX:  val = scx;  break;
        case LY:   val = ly;   break;
        case LYC:  val = lyc;  break;
        case BGP:  val = bgp;  break;
        case OBP0: val = obp0; break;
        case OBP1: val = obp1; break;
        case WY:   val = wy;   break;
        case WX:   val = wx;   break;
        default:   return false;
    }

    return true;
}

bool PPU::regWrite(uint16_t addr, uint8_t data) {
    switch(addr) {
        case LCDC: lcdc = data; break;
        case STAT: stat = data; break;
        case SCY:  scy = data;  break;
        case SCX:  scx = data;  break;
        case LY:   ly = 0;      break;
        case LYC:  lyc = data;  break;
        case BGP:  bgp = data;  break;
        case OBP0: obp0 = data; break;
        case OBP1: obp1 = data; break;
        case WY:   wy = data;   break;
        case WX:   wx = data;   break;
        default:   return false;
    }

    return true;
}

uint8_t PPU::read(uint16_t addr) {
    if (addr >= 0x8000 && addr < 0xA000) {
        return vram[addr & 0x7FFF];
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        return oam[addr & 0x01FF];
    }

    return 0;
}

void PPU::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x8000 && addr < 0xA000) {
        vram[addr & 0x7FFF] = data;
    } else if (addr >= 0xFE00 && addr < 0xFEA0) {
        oam[addr & 0x00FF] = data;
     }
}

void PPU::connectBus(Bus *bus) {
    this->bus = bus;
}