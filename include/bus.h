#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "cpu.h"
#include "ppu.h"
#include "cartridge.h"
#include "interrupt.h"
#include "timer.h"
#include "gb_driver.h"

#define KB 1024
#define POLL_INTERVAL 210672

class Bus {
public:
    Bus(GameboyDriver *driver);
    ~Bus() = default;

private:
    CPU cpu;
    PPU ppu;
    Timer timer;
    std::shared_ptr<Cartridge> cart;
    std::array<uint8_t, 8 * KB> ram;
    std::array<uint8_t, 0xFF> zero_page_ram;

    GameboyDriver *driver;

public:
    uint8_t cpuRead(uint16_t addr);
    void cpuWrite(uint16_t addr, uint8_t data);

    uint8_t timerRead(uint16_t addr);
    void timerWrite(uint16_t addr, uint8_t data);

    uint8_t ppuRead(uint16_t addr);
    void ppuWrite(uint16_t addr, uint8_t data);

    void requestInterrupt(INTERRUPT intr);

    void reset();

    void run();
    
    void insertCartridge(const std::shared_ptr<Cartridge> cart);
};
