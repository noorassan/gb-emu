#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "cartridge.h"
#include "controls.h"
#include "cpu.h"
#include "ppu.h"
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
    Controls controls;
    std::shared_ptr<Cartridge> cart;
    std::array<uint8_t, 8 * KB> ram;
    std::array<uint8_t, 0xFF> zero_page_ram;

    GameboyDriver *driver;

public:
    uint8_t cpuRead(uint16_t addr);
    void cpuWrite(uint16_t addr, uint8_t data);

    // For timer, PPU. controls, etc.
    // Allows access to high ram without going through devices in charge of registers
    uint8_t deviceRead(uint16_t addr);
    void deviceWrite(uint16_t addr, uint8_t data);

    void requestInterrupt(INTERRUPT intr);

    void reset();

    void run();
    
    void insertCartridge(const std::shared_ptr<Cartridge> cart);
};
