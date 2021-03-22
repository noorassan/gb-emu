#pragma once

#include <array>
#include <cstdint>

#include "gb_driver.h"

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
    PPU(GameboyDriver *driver);
    ~PPU() = default;

public:
    // Handle reads and writes coming from the CPU
    uint8_t cpuRead(uint16_t addr);
    void cpuWrite(uint16_t addr, uint8_t data);
    bool handlesAddr(uint16_t addr);

    void reset();
    void clock(uint8_t clocks);
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
            unsigned unlit: 1; // If the bg/win are disabled they display as actual white
        };
        uint8_t data;
    } Pixel;

    typedef struct {
        uint8_t pos_y, pos_x;
        uint8_t tile_num;
        uint8_t flags;
    } Sprite;

private:
    uint32_t cycles;

    // the number of cycles spent on PIXEL_TRANSFER determines 
    // the length of H_BLANK, so we keep track of it
    uint32_t transfer_cycles;

    std::array<uint8_t, 8 * KB> vram;
    std::array<uint8_t, 160> oam;

    std::vector<Pixel> pixel_line;
    std::array<Sprite, 10> sprites;

    Bus *bus;
    GameboyDriver *driver;

private:
    // Handle reads and writes from the PPU itself
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    PPU_STATUS getStatus();
    void setStatus(PPU_STATUS status);

    bool isPPUEnabled();
    bool isWinEnabled();
    bool isBGEnabled();
    bool isOBJEnabled();

    void fetchLine();

    // Fetches the first num_pixels pixels of the bg for line
    void fetchBG(uint8_t line, uint8_t num_pixels);
    void fetchWin(uint8_t line);
    void fetchOBJ();

    void drawLine();

    void clockPixelFIFO();
    void statInterrupt();

    uint16_t getBGTilemapStart();
    uint16_t getWinTilemapStart();

    void fetchTileLine(uint8_t tile_id, uint8_t tile_line, std::array<Pixel, 8> &out);
};