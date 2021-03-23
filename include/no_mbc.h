#pragma once

#include "mbc.h"


class NoMBC : public MemoryBankController {
public:
    NoMBC() = default;
    ~NoMBC() = default;

public:
    bool read(uint16_t addr, uint32_t &mapped_addr, bool &rom_read) override;
    bool write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) override;
};