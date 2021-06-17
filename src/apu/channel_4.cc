#include "apu/apu_addrs.h"
#include "apu/channel_4.h"

Channel4::Channel4() {
	reset();
}

bool Channel4::regRead(uint16_t addr, uint8_t &val) {
    switch(addr) {
        case NR41: val = nr41;   break;
        case NR42: val = nr42;   break;
        case NR43: val = nr43;   break;
        case NR44: val = nr44;   break;
        default:   return false; break;
    }

    return true;
}

bool Channel4::regWrite(uint16_t addr, uint8_t data) {
    switch(addr) {
        case NR41: nr41 = data;  break;
        case NR42: nr42 = data;  break;
        case NR43: nr43 = data;  break;
        case NR44: nr44 = data;  break;
        default:   return false; break;
    }

    return true;
}

void Channel4::reset() {
    enabled = true;

	return;
}

void Channel4::clock(uint8_t clocks) {
	return;
}

uint8_t Channel4::getOutput() {
	return 0;
}

bool Channel4::isEnabled() {
    return enabled;
}

void Channel4::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void Channel4::trigger() {
    return;
}