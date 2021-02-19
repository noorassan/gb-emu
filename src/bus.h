#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "cpu.h"
#include "cartridge.h"

#define KB 1024

class Bus {
public:
    Bus();
    ~Bus() = default;

private:
    CPU cpu;
    std::shared_ptr<Cartridge> cart;
    std::array<uint8_t, 64 * KB> ram;

public:
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    void reset();
    void clock();
    
    void insertCartridge(const std::shared_ptr<Cartridge> cart);
};