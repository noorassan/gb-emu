#include "apu/apu_addrs.h"
#include "apu/channel_2.h"

Channel2::Channel2() : len_counter(this) {
    duty_cycles = {0xF0, 0x81, 0xE1, 0x7E};

	reset();
}

bool Channel2::regRead(uint16_t addr, uint8_t &val) {
    switch(addr) {
        case NR21: val = nr21;   break;
        case NR22: val = nr22;   break;
        case NR23: val = nr23;   break;
        case NR24: val = nr24;   break;
        default:   return false; break;
    }

    return true;
}

bool Channel2::regWrite(uint16_t addr, uint8_t data) {
    switch(addr) {
        case NR21: nr21 = data;  break;
        case NR22: nr22 = data;  break;
        case NR23: nr23 = data;  break;
        case NR24:
            if (data & 0x80) trigger();
            nr24 = data;
            break;
        default:   return false; break;
    }

    return true;
}

void Channel2::reset() {
    nr21 = 0xBF;
    nr22 = 0xF3;
    nr24 = 0xBF;

    duty_pointer = 0;
    duty_timer = SQUARE_FREQ_TO_PERIOD(getFrequency());
    enabled = true;

    len_counter.reset();
}

void Channel2::clock(uint8_t clocks) {
    uint32_t period = SQUARE_FREQ_TO_PERIOD(getFrequency());
    if (clocks >= duty_timer) {
        duty_pointer += 1 + ((clocks - duty_timer) / period);
        duty_pointer %= 8;

        duty_timer = period - ((clocks - duty_timer) % period);
    } else {
        duty_timer -= clocks;
    }

    len_counter.clock(clocks);
}

uint8_t Channel2::getOutput() {
    return ((duty_cycles[getDuty()] >> duty_pointer) & 0x01) * getVolume();
}

bool Channel2::isEnabled() {
    return enabled;
}

void Channel2::setEnabled(bool enabled) {
    this->enabled = enabled;
}

uint8_t Channel2::getLength() {
    return nr21 & 0x3F;
}

void Channel2::setLength(uint8_t length) {
    nr21 = (nr21 & 0xC0) | (length & 0x3F); 
}

bool Channel2::isLengthEnabled() {
    return nr24 & 0x40;
}

void Channel2::setLengthEnabled(bool enabled) {
    if (enabled) {
        nr24 |= 0x40;
    } else {
        nr24 &= 0xBF;
    }
}

void Channel2::trigger() {
    setEnabled(true);

    duty_timer = SQUARE_FREQ_TO_PERIOD(getFrequency());

    len_counter.trigger();
}

uint16_t Channel2::getFrequency() {
     return nr23 + ((nr24 & 0x07) << 8);
 }

uint8_t Channel2::getVolume() {
    return (nr22 & 0xF0) >> 4;
}

 uint8_t Channel2::getDuty() {
     return (nr21 & 0xC0) >> 6;
 }