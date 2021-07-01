#include "apu/apu_addrs.h"
#include "apu/channel_4.h"

Channel4::Channel4() : len_counter(this), envelope(this) {
    divisors = {8, 16, 32, 48, 64, 80, 96, 112};

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
        case NR44:
            if (data & 0x80) trigger();
            nr44 = data;
            break;
        default:   return false; break;
    }

    return true;
}

void Channel4::reset() {
    enabled = true;

    timer = getDivisor() << getClockShift();
    lfsr = 0x7FFF;

    len_counter.reset();
    envelope.reset();
}

void Channel4::clock(uint8_t clocks) {
    uint32_t period = getDivisor() << getClockShift();
    if (clocks >= timer) {
        uint8_t xor_result = (lfsr & 0x0001) ^ ((lfsr & 0x0002) >> 1);
        lfsr >>= 1;
        lfsr |= xor_result << 14;

        if (isWidthModeEnabled()) {
            lfsr &= 0x003F;
            lfsr |= xor_result << 6;
        }

        timer = period - ((clocks - timer) % period);
    } else {
        timer -= clocks;
    }

    len_counter.clock(clocks);
    envelope.clock(clocks);
}

uint8_t Channel4::getOutput() {
    return ((~lfsr) & 0x0001) * envelope.getVolume();
}

bool Channel4::isEnabled() {
    return enabled;
}

void Channel4::setEnabled(bool enabled) {
    this->enabled = enabled;
}

uint8_t Channel4::getLength() {
    return nr41 & 0x3F;
}

void Channel4::setLength(uint8_t length) {
    nr41 = (nr41 & 0xC0) | (length & 0x3F);
}

bool Channel4::isLengthEnabled() {
    return nr44 & 0x40;
}

void Channel4::setLengthEnabled(bool enabled) {
    if (enabled) {
        nr44 |= 0x40;
    } else {
        nr44 &= 0xBF;
    }
}

uint8_t Channel4::getVolume() {
    return (nr42 & 0xF0) >> 4;
}

uint8_t Channel4::getEnvelopePeriod() {
    return nr42 & 0x07;
}

bool Channel4::isAddModeEnabled() {
    return nr42 & 0x08;
}

void Channel4::trigger() {
    enabled = true;

    timer = getDivisor() << getClockShift();
    lfsr = 0x7FFF;

    len_counter.trigger();
    envelope.trigger();
}

uint8_t Channel4::getClockShift() {
    return (nr43 & 0xF0) >> 4;
}

uint8_t Channel4::getDivisor() {
    return divisors[nr43 & 0x07];
}

bool Channel4::isWidthModeEnabled() {
    return nr43 & 0x08;
}