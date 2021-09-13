#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <fstream>

#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "apu/apu.h"
#include "timer.h"
#include "controls.h"
#include "gb_driver.h"
#include "interrupt.h"

#define POLL_INTERVAL 210672

#define DMA 0xFF46
#define SB 0xFF01 // serial bus
#define SC 0xFF02 // serial control
#define IF 0xFF0F // interrupt flag


class Bus {
    public:
        Bus(GameboyDriver *driver);
        ~Bus() = default;

    private:
        CPU cpu;
        PPU ppu;
        APU apu;
        Timer timer;
        Controls controls;
        std::shared_ptr<Cartridge> cart;
        std::array<uint8_t, 8 * KB> ram;
        std::array<uint8_t, 0x7F> high_ram;

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
        void handleDMA(uint8_t data);

        void handleIOWrite(uint16_t addr, uint8_t data);
        uint8_t handleIORead(uint16_t addr);

    private:
        uint8_t sb;
        uint8_t sc;
        uint8_t intr_flag;
};
