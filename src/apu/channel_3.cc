
#include "apu/apu_addrs.h"
#include "apu/channel_3.h"

Channel3::Channel3() {
	reset();
}

bool Channel3::regRead(uint16_t addr, uint8_t &val) {
	if (addr >= WAVE_PATTERN_START && addr < WAVE_PATTERN_END) {
        val = wave_pattern_ram[addr & 0x000F];
        return true;
    }

    switch(addr) {
        case NR30: val = nr30;   break;
        case NR31: val = nr31;   break;
        case NR32: val = nr32;   break;
        case NR33: val = nr33;   break;
        case NR34: val = nr34;   break;
        default:   return false; break;
    }

    return true;
}

bool Channel3::regWrite(uint16_t addr, uint8_t data) {
	if (addr >= WAVE_PATTERN_START && addr < WAVE_PATTERN_END) {
        wave_pattern_ram[addr & 0x000F] = data;
        return true;
    }

    switch(addr) {
        case NR30: nr30 = data;  break;
        case NR31: nr31 = data;  break;
        case NR32: nr32 = data;  break;
        case NR33: nr33 = data;  break;
        case NR34: nr34 = data;  break;
        default:   return false; break;
    }

    return true;
}

void Channel3::reset() {
    enabled = true;

    return;
}

void Channel3::clock(uint8_t clocks) {
    return;
}

uint8_t Channel3::getOutput() {
    return 0;
}

bool Channel3::isEnabled() {
    return enabled;
}

void Channel3::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void Channel3::trigger() {
    return;
}
