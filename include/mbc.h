#pragma once

#include <cstdint>


class MemoryBankController {
public:
    virtual ~MemoryBankController() = default;

public:
    virtual bool read(uint16_t addr, uint32_t &mapped_addr, bool &rom_read) = 0;
    virtual bool write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) = 0;
};