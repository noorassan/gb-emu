#pragma once

#include <array>
#include <cstdint>

#include "color.h"
#include "bus_io_fns.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

#define KB 1024

// Important PPU ram locations
#define LCDC 0xFF40
#define STAT 0xFF41
#define SCY 0xFF42
#define SCX 0xFF43
#define LY 0xFF44
#define LYC 0xFF45
#define DMA 0xFF46
#define BGP 0xFF47
#define OBP0 0xFF48
#define OBP1 0xFF49
#define WY 0xFF4A
#define WX 0xFF4B

class Bus;

class PPU {
public:
    // PPU must be instantiated with a function that allows it to output pixels
    PPU(DRAW);
    ~PPU() = default;

public:
    // Handle reads and writes coming from the bus
    uint8_t busRead(uint16_t addr);
    void busWrite(uint16_t addr, uint8_t data);

    void reset();
    void clock();
    void connectBus(Bus *bus);

private:
    enum PPU_STATUS {
        H_BLANK,
        V_BLANK,
        OAM_SEARCH,
        PIXEL_TRANSFER,
    };

    typedef union {
        struct {
            unsigned color : 2;
            unsigned bgp : 1;
            unsigned obp0 : 1;
            unsigned obp1 : 1;
            unsigned bg_priority : 1; // 0 = obj above bg; 1 = obj behind bg color 1-3
        };
        uint8_t data;
    } Pixel;

    struct SPRITE {
        uint8_t pos_y, pos_x;
        uint8_t tile_num;
        uint8_t flags;
    };

private:
    uint8_t cycles;
    uint8_t fetch_cycles;

    std::array<uint8_t, 8 * KB> vram;
    std::array<uint8_t, 160> oam;

    Bus *bus;

    std::vector<Pixel> pixel_fifo;
    std::vector<Pixel> pending_fifo;
    std::array<SPRITE, 10> sprites;

    // Describes position on screen to output to next
    uint8_t x_pos;

    // Describes tile to be fetched next
    uint8_t tile_num;
    
    uint8_t skip_pixels;

    bool new_status;

private:
    // Handle reads and writes from the PPU itself
    uint8_t ppuRead(uint16_t addr);
    void ppuWrite(uint16_t addr, uint8_t data);

    void clockPixelFIFO();

    PPU_STATUS getStatus();
    void setStatus(PPU_STATUS status);

    uint8_t getLY();
    void setLY(uint8_t val);

    uint16_t getBGTilemapStart();
    uint16_t getWinTilemapStart();
    void fetchTileLine(uint8_t tile_id);

    uint8_t fetchPixels();
    
    void (*draw)(COLOR color, uint8_t x, uint8_t y);
};