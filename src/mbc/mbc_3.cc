#include <algorithm>

#include "bit_utils.h"
#include "mbc/mbc_3.h"

MBC3::MBC3(uint16_t rom_banks, uint8_t ram_banks) : MBC(rom_banks, ram_banks) {
    ram_rtc_enabled = false;

    ram_rtc_bank = 0;
    rom_bank = 0x01;

    last_latch_write = 0xFF;

    rtc_s = 0; rtc_m = 0; rtc_h = 0; rtc_dl = 0; rtc_dh = 0;
}

bool MBC3::read(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) {
    if (addr >= 0x0000 && addr < 0x4000) {
        // Read from ROM bank 0
        mapped_addr = addr;
        return true;
    } else if (addr >= 0x4000 && addr < 0x8000) {
        // Read from currently selected ROM bank
        mapped_addr = mapROMAddress(addr, rom_bank & (highestOrderBit(rom_banks) - 1));
        return true;
    } else if (addr >= 0xA000 && addr < 0xC000 && ram_rtc_enabled) {
        // Read from currently selected RAM/RTC bank
        if (ram_rtc_enabled) {
            if (ram_rtc_bank <= 0x03) {
                mapped_addr = mapRAMAddress(addr, ram_rtc_bank & (highestOrderBit(ram_banks) - 1));
                return true;
            } else if (ram_rtc_bank <= 0x0C) {
                data = getRTCData();
                return false;
            }
        }
    }

    data = 0xFF;
    return false;
}

bool MBC3::write(uint16_t addr, uint8_t data, uint32_t &mapped_addr) {
    if (addr >= 0x0000 && addr < 0x2000) {
        // Enable/disable RAM
        ram_rtc_enabled = (data & 0x0F) == 0x0A;
    } else if (addr >= 0x2000 && addr < 0x4000) {
        // Select ROM bank
        // ROM bank 0 cannot be selected since it is always available
        rom_bank = std::max(1, data & 0x7F);
    } else if (addr >= 0x4000 && addr < 0x6000) {
        // Select RAM bank/RTC bank
        ram_rtc_bank = data;
    } else if (addr >= 0x6000 && addr < 0x8000) {
        // Latch clock data
        // Only latch if 0x00 written previously and 0x01 written this time
        if (!last_latch_write && data == 0x01) {
            rtc_s = 0; rtc_m = 0; rtc_h = 0; rtc_dl = 0; rtc_dh = 0;
        }

        last_latch_write = data;
    } else if (addr >= 0xA000 && addr < 0xC000) {
        // Write to RAM or RTC
        if (ram_rtc_enabled) {
            if (ram_rtc_bank <= 0x03) {
                mapped_addr = mapRAMAddress(addr, ram_rtc_bank & (highestOrderBit(ram_banks) - 1));
                return true;
            } else if (ram_rtc_bank <= 0x0C) {
                setRTCData(data);
            }
        }
    }

    return false;
}

uint8_t MBC3::getRTCData() {
    switch(ram_rtc_bank) {
        case 0x08:
            return rtc_s;
        case 0x09:
            return rtc_m;
        case 0x0A:
            return rtc_h;
        case 0x0B:
            return rtc_dl;
        case 0x0C:
            return rtc_dh;
    }

    return 0x00;
}

void MBC3::setRTCData(uint8_t data) {
    switch(ram_rtc_bank) {
        case 0x08:
            rtc_s = data;
        case 0x09:
            rtc_m = data;
        case 0x0A:
            rtc_h = data;
        case 0x0B:
            rtc_dl = data;
        case 0x0C:
            rtc_dh = data;
    }
}