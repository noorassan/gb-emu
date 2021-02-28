#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include "mbc.h"

class Cartridge {
public:
    Cartridge(const std::string &filename);
    ~Cartridge() = default;

private:
    std::string title;

    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;

    std::shared_ptr<MemoryBankController> mbc;

public:
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    std::string getTitle();

private:
    void setROMSize();
    void setRAMSize();
    void setMBC();
};