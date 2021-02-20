#pragma once

#include "mbc.h"

class NoMBC : public MemoryBankController {
public:
    NoMBC() = default;
    ~NoMBC() = default;

public:
    bool read(uint16_t addr, uint16_t &mapped_addr) override;
    bool write(uint16_t addr, uint8_t data, uint16_t &mapped_addr) override;
};