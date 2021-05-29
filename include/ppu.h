#pragma once

#include <array>
#include <cstdint>

#include "gb_driver.h"
#include "sprite.h"

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
#define OAM_START 0xFE00
#define OAM_END 0xFEA0
#define OAM_SIZE 0xA0

class Bus;


class PPU {
public:
    // PPU must be instantiated with a function that allows it to output pixels
    PPU(GameboyDriver *driver);
    ~PPU() = default;

public:
    // Handles reads and writes coming from the CPU accessing VRAM & OAM
    void cpuWrite(uint16_t addr, uint8_t data);
    uint8_t cpuRead(uint16_t addr);

    // Handles reads/writes to PPU high ram registers
    bool regWrite(uint16_t addr, uint8_t data);
    bool regRead(uint16_t addr, uint8_t &val);

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

private:
    uint32_t cycles;

    // the number of cycles spent on PIXEL_TRANSFER determines 
    // the length of H_BLANK, so we keep track of it
    uint32_t transfer_cycles;

    std::array<uint8_t, 8 * KB> vram;
    std::array<uint8_t, OAM_SIZE> oam;

    std::vector<Pixel> pixel_line;
    std::vector<Sprite> sprites;

    Bus *bus;
    GameboyDriver *driver;

    // PPU registers
    uint8_t lcdc;
    uint8_t stat;
    uint8_t scy;
    uint8_t scx;
    uint8_t ly;
    uint8_t lyc;
    uint8_t bgp;
    uint8_t obp0;
    uint8_t obp1;
    uint8_t wy;
    uint8_t wx;

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

    void searchOAM(uint8_t line);

    void fetchLine();

    // Fetches the first num_pixels pixels of the bg for line
    void fetchBG(uint8_t line, uint8_t num_pixels);

    // Fetched the first num_pixels pixels of the window for line
    void fetchWin(uint8_t win_line, uint8_t num_pixels);

    // Overlay sprites onto current line of pixels
    void fetchOBJ(uint8_t line);

    void drawLine();
    
    void checkSTATInterrupt();

    uint16_t getBGTilemapStart();
    uint16_t getWinTilemapStart();
    uint8_t getOBJHeight();

    void fetchTileLine(uint8_t tile_id, uint8_t tile_line, std::array<Pixel, 8> &out);
    void fetchOBJLine(const Sprite &sprite, uint8_t curr_line, std::array<Pixel, 8> &out);
    void decodePixels(uint8_t low_byte, uint8_t high_byte, std::array<Pixel, 8> &out);

};