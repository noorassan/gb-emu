
#include "apu/apu_addrs.h"
#include "apu/channel_3.h"

Channel3::Channel3() : len_counter(this) {
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
        case NR34:
            if (data & 0x80) trigger();
            nr34 = data;
            break;
        default:   return false; break;
    }

    return true;
}

void Channel3::reset() {
    nr30 = 0x7F;
    nr31 = 0xFF;
    nr32 = 0x9F;
    nr33 = 0xBF;

    table_timer = WAVE_FREQ_TO_PERIOD(getFrequency());
    table_pointer = 0;
    sample = wave_pattern_ram[0];
    enabled = true;

    len_counter.reset();
}

void Channel3::clock(uint8_t clocks) {
    uint32_t period = WAVE_FREQ_TO_PERIOD(getFrequency());
    if (clocks >= table_timer) {
        table_pointer += 1 + ((clocks - table_timer) / period);
        table_pointer %= 32;
        sample = wave_pattern_ram[table_pointer / 2];

        table_timer = period - ((clocks - table_timer) % period);
    } else {
        table_timer -= clocks;
    }

    len_counter.clock(clocks);
}

uint8_t Channel3::getOutput() {
    uint8_t output;

    if (table_pointer % 2) {
        output = sample & 0x0F;
    } else {
        output = (sample & 0xF0) >> 4;
    }

    uint8_t volume_code = getVolumeCode();

    switch(volume_code) {
        case 0:
            output >>= 4;
            break;
        case 1:
            output >>= 0;
            break;
        case 2:
            output >>= 1;
            break;
        case 3:
            output >>= 2;
            break;
        default:
            throw std::runtime_error("Invalid value returned from Channel3::getVolumeCode().");
    }

    return output;
}

bool Channel3::isEnabled() {
    return enabled;
}

void Channel3::setEnabled(bool enabled) {
    this->enabled = enabled;
}

uint8_t Channel3::getLength() {
    return nr31 & 0x3F;
}

void Channel3::setLength(uint8_t length) {
    nr31 = (nr31 & 0xC0) | (length & 0x3F);
}

bool Channel3::isLengthEnabled() {
    return nr34 & 0x40;
}

void Channel3::setLengthEnabled(bool enabled) {
    if (enabled) {
        nr34 |= 0x40;
    } else {
        nr34 &= 0xBF;
    }
}

void Channel3::trigger() {
    setEnabled(true);

    table_timer = WAVE_FREQ_TO_PERIOD(getFrequency());
    table_pointer = 0;

    len_counter.trigger();
}

uint16_t Channel3::getFrequency() {
     return nr33 + ((nr34 & 0x07) << 8);
 }

uint8_t Channel3::getVolumeCode() {
    return (nr32 & 0x60) >> 5;
}