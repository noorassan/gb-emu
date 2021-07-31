#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <fstream>

#include "mbc/mbc.h"

#define KB 1024


class Cartridge {
public:
    Cartridge(const std::string &filename);
    ~Cartridge() = default;

private:
    std::string title;

    uint16_t rom_banks;
    uint8_t ram_banks;

    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;

    std::shared_ptr<MBC> mbc;

public:
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    void saveRAM(std::ofstream &ofs);
    void loadRAM(std::ifstream &ifs);

    std::string getTitle();

private:
    void setROMSize();
    void setRAMSize();
    void setMBC();
};