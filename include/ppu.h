#pragma once

#include <array>
#include<cstdint>

#define KB 1024

class Bus;

class PPU {
public:
    PPU() = default;
    ~PPU() = default;

private:
    std::array<uint8_t, 8 * KB> vram;
    std::array<uint8_t, 160> oam;

public:
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    void clock();
};