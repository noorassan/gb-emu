#pragma once

class MemoryBankController {
public:
    virtual ~MemoryBankController() = default;

public:
    virtual bool read(uint16_t addr, uint16_t &mapped_addr, bool &rom_read) = 0;
    virtual bool write(uint16_t addr, uint8_t data, uint16_t &mapped_addr) = 0;
};