#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <fstream>

#include "cartridge.h"
#include "controls.h"
#include "cpu.h"
#include "gb_driver.h"
#include "interrupt.h"
#include "ppu.h"
#include "timer.h"

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

    void requestInterrupt(INTERRUPT intr);

    void reset();

    void run();

    void insertCartridge(const std::shared_ptr<Cartridge> cart);

    void saveState(const std::string &filename);
    void loadState(const std::string &filename);

private:
    bool handleDMA(uint16_t addr, uint8_t data);
};
