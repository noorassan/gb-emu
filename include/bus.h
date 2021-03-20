#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "bus_io_fns.h"
#include "cpu.h"
#include "ppu.h"
#include "cartridge.h"
#include "interrupt.h"
#include "timer.h"
#include "controls.h"

#define KB 1024

class Bus {
public:
    Bus(DrawFn draw, PollControlsFn poll_controls);
    ~Bus() = default;

private:
    CPU cpu;
    PPU ppu;
    Timer timer;
    std::shared_ptr<Cartridge> cart;
    std::array<uint8_t, 8 * KB> ram;
    std::array<uint8_t, 0xFF> zero_page_ram;

    uint32_t cycles;
    uint8_t poll_state;

    PollControlsFn poll;

public:
    uint8_t cpuRead(uint16_t addr);
    void cpuWrite(uint16_t addr, uint8_t data);

    uint8_t timerRead(uint16_t addr);
    void timerWrite(uint16_t addr, uint8_t data);

    uint8_t ppuRead(uint16_t addr);
    void ppuWrite(uint16_t addr, uint8_t data);

    void requestInterrupt(INTERRUPT intr);

    void reset();

    // Returns true on encountering QUIT and false otherwise
    bool clock(uint32_t clocks);
    
    void insertCartridge(const std::shared_ptr<Cartridge> cart);
};
