#include "apu/apu_addrs.h"
#include "apu/channel_1.h"

Channel1::Channel1() : len_counter(this) {
    duty_cycles = {0xF0, 0x81, 0xE1, 0x7E};

	reset();
}

bool Channel1::regRead(uint16_t addr, uint8_t &val) {
    switch(addr) {
        case NR10: val = nr10;   break;
        case NR11: val = nr11;   break;
        case NR12: val = nr12;   break;
        case NR13: val = nr13;   break;
        case NR14: val = nr14;   break;
        default:   return false; break;
    }

    return true;
}

bool Channel1::regWrite(uint16_t addr, uint8_t data) {
    switch(addr) {
        case NR10: nr10 = data;  break;
        case NR11: nr11 = data;  break;
        case NR12: nr12 = data;  break;
        case NR13: nr13 = data;  break;
        case NR14:
            if (data & 0x80) trigger();
            nr14 = data;
            break;
        default:   return false; break;
    }

    return true;
}

void Channel1::reset() {
    nr10 = 0x80;
    nr11 = 0xBF;
    nr12 = 0xF3;
    nr14 = 0xBF;

    duty_pointer = 0;
    duty_timer = FREQ_TO_PERIOD(getFrequency());
    enabled = true;

    len_counter.reset();
}

void Channel1::clock(uint8_t clocks) {
    uint32_t period = FREQ_TO_PERIOD(getFrequency());
    if (clocks >= duty_timer) {
        duty_pointer += 1 + ((clocks - duty_timer) / period);
        duty_pointer %= 8;

        duty_timer = period - ((clocks - duty_timer) % period);
    } else {
        duty_timer -= clocks;
    }

    len_counter.clock(clocks);
}

uint8_t Channel1::getOutput() {
    return ((duty_cycles[getDuty()] >> duty_pointer) & 0x01) * getVolume();
}

bool Channel1::isEnabled() {
    return enabled;
}

void Channel1::setEnabled(bool enabled) {
    this->enabled = enabled;
}

uint16_t Channel1::getLength() {
    return nr11 & 0x3F;
}

void Channel1::setLength(uint16_t length) {
    nr11 = (nr11 & 0xC0) | (length & 0x3F); 
}

bool Channel1::isLengthEnabled() {
    return nr14 & 0x40;
}

void Channel1::setLengthEnabled(bool enabled) {
    if (enabled) {
        nr14 |= 0x40;
    } else {
        nr14 &= 0xBF;
    }
}

void Channel1::trigger() {
    setEnabled(true);

    if (!getLength()) {
        setLength(0x40);
    }

    // volume envelope timer reloaded
    // reload volume from NR12

    // lots of things happen with frequency sweep
}

uint16_t Channel1::getFrequency() {
     return nr13 + ((nr14 & 0x07) << 8);
 }

uint8_t Channel1::getVolume() {
    return (nr12 & 0xF0) >> 4;
}

 uint8_t Channel1::getDuty() {
     return (nr11 & 0xC0) >> 6;
 }