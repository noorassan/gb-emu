#include <algorithm>

#include "apu.h"
#include "audio_output.h"

APU::APU(GameboyDriver *driver) {
    this->driver = driver;
    duty_cycles = {0xF0, 0x81, 0xE1, 0x7E};
    sample_frequency = GB_CLOCK_RATE / this->driver->getSamplingRate();

    reset();
}

void APU::reset() {
    // register initial values
    nr10 = 0x80; nr11 = 0xBF; nr12 = 0xF3; nr14 = 0xBF; nr21 = 0x3F; nr22 = 0x00; 
    nr24 = 0xBF; nr30 = 0x7F; nr31 = 0xFF; nr32 = 0x9F; nr33 = 0xBF; nr41 = 0xFF; 
    nr42 = 0x00; nr43 = 0x00; nr44 = 0xBF; nr50 = 0x77; nr51 = 0xF3; nr52 = 0xF3;

    resetCh1();
    resetCh2();

    clocks_to_sample = 0;
}

void APU::clock(uint8_t clocks) {
    if (isAPUEnabled()) {
        // perform as many clocks as possible at each step until we need to provide a sample
        while (clocks >= clocks_to_sample) {
            AudioOutput output = {};

            clockCh1(clocks_to_sample);
            clockCh2(clocks_to_sample);

            clocks -= clocks_to_sample;
            clocks_to_sample = sample_frequency;

            // mix channel 1
            if (isCh1Enabled()) {
                uint8_t ch1_output = getCh1Output();

                // output left
                if (nr51 & 0x10) {
                    output.ch1_left = ch1_output * getLeftVolume();
                }
                // output right
                if (nr51 & 0x01) {
                    output.ch1_right = ch1_output * getRightVolume();
                }
            }

            // mix channel 2
            if (isCh2Enabled()) {
                uint8_t ch2_output = ((getCh2Duty() >> ch2_pointer) & 0x01) * getCh2Volume();

                // output left
                if (nr51 & 0x20) {
                    output.ch2_left = ch2_output * getLeftVolume();
                }
                // output right
                if (nr51 & 0x02) {
                    output.ch2_right = ch2_output * getRightVolume();
                }
            }

            driver->pushSample(output);
        }

        // apply remaining clocks
        clockCh1(clocks);
        clockCh2(clocks);
        clocks_to_sample -= clocks;
    }
}

bool APU::isAPUEnabled() {
    return nr52 & 0x80;
}

uint8_t APU::getLeftVolume() {
    return (nr50 >> 4) & 0x07;
}

uint8_t APU::getRightVolume() {
    return (nr50 >> 0) & 0x07;
}

bool APU::regWrite(uint16_t addr, uint8_t data) {
    if (addr >= WAVE_PATTERN_START && addr < WAVE_PATTERN_END) {
        wave_pattern_ram[addr & 0x000F] = data;
        return true;
    }

    switch(addr) {
        case NR10: nr10 = data; break; 
        case NR11: nr11 = data; break;
        case NR12: nr12 = data; break;
        case NR13: nr13 = data; break;
        case NR14: nr14 = data; break;

        case NR21: nr21 = data; break;
        case NR22: nr22 = data; break;
        case NR23: nr23 = data; break;
        case NR24: nr24 = data; break;

        case NR30: nr30 = data; break;
        case NR31: nr31 = data; break;
        case NR32: nr32 = data; break;
        case NR33: nr33 = data; break;
        case NR34: nr34 = data; break;

        case NR41: nr41 = data; break;
        case NR42: nr42 = data; break;
        case NR43: nr43 = data; break;
        case NR44: nr44 = data; break;

        case NR50: nr50 = data; break;
        case NR51: nr51 = data; break;
        case NR52: 
            nr52 = (nr52 & 0x7F) | (data & 0x80); 
            break;
        default:   return false;
    }
    
    return true;
}

bool APU::regRead(uint16_t addr, uint8_t &val) {
    if (addr >= WAVE_PATTERN_START && addr < WAVE_PATTERN_END) {
        val = wave_pattern_ram[addr & 0x000F];
        return true;
    }

    switch(addr) {
        case NR10: val = nr10; break; 
        case NR11: val = nr11; break;
        case NR12: val = nr12; break;
        // NR13 is write-only
        case NR14: val = nr14; break;

        case NR21: val = nr21; break;
        case NR22: val = nr22; break;
        // NR23 is write-only
        case NR24: val = nr24; break;

        case NR30: val = nr30; break;
        case NR31: val = nr31; break;
        case NR32: val = nr32; break;
        // NR33 is write-only
        case NR34: val = nr34; break;

        case NR41: val = nr41; break;
        case NR42: val = nr42; break;
        case NR43: val = nr43; break;
        case NR44: val = nr44; break;

        case NR50: val = nr50; break;
        case NR51: val = nr51; break;
        case NR52: val = nr52; break;
        default:   return false;
    }

    return true;
}
