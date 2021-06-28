#include <algorithm>

#include "apu/apu.h"
#include "audio_output.h"

APU::APU(GameboyDriver *driver) {
    this->driver = driver;
    duty_cycles = {0xF0, 0x81, 0xE1, 0x7E};
    sample_frequency = GB_CLOCK_RATE / this->driver->sampling_rate;

    reset();
}

void APU::reset() {
    // register initial values
    nr50 = 0x77;
    nr51 = 0xF3;
    nr52 = 0xF3;

    ch1.reset();
    ch2.reset();
    ch3.reset();
    ch4.reset();

    clocks_to_sample = 0;
}

void APU::clock(uint8_t clocks) {
    if (isAPUEnabled()) {
        // perform as many clocks as possible at each step until we need to provide a sample
        while (clocks >= clocks_to_sample) {
            AudioOutput output = {};

            ch1.clock(clocks_to_sample);
            ch2.clock(clocks_to_sample);
            ch3.clock(clocks_to_sample);
            ch4.clock(clocks_to_sample);

            clocks -= clocks_to_sample;
            clocks_to_sample = sample_frequency;

            // mix channel 1
            if (ch1.isEnabled()) {
                uint8_t ch1_output = ch1.getOutput();

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
            if (ch2.isEnabled()) {
                uint8_t ch2_output = ch2.getOutput();

                // output left
                if (nr51 & 0x20) {
                    output.ch2_left = ch2_output * getLeftVolume();
                }
                // output right
                if (nr51 & 0x02) {
                    output.ch2_right = ch2_output * getRightVolume();
                }
            }

            // mix channel 3
            if (ch3.isEnabled()) {
                uint8_t ch3_output = ch3.getOutput();

                // output left
                if (nr51 & 0x04) {
                    output.ch3_left = ch3_output * getLeftVolume();
                }
                // output right
                if (nr51 & 0x40) {
                    output.ch3_right = ch3_output * getRightVolume();
                }
            }

            // mix channel 4
            if (ch4.isEnabled()) {
                uint8_t ch4_output = ch4.getOutput();

                // output left
                if (nr51 & 0x08) {
                    output.ch4_left = ch4_output * getLeftVolume();
                }
                // output right
                if (nr51 & 0x80) {
                    output.ch4_right = ch4_output * getRightVolume();
                }
            }

            driver->pushSample(output);
        }

        // apply remaining clocks
        ch1.clock(clocks);
        ch2.clock(clocks);
        ch3.clock(clocks);
        ch4.clock(clocks);
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
    if (ch1.regWrite(addr, data)) {
        return true;
    }
    if (ch2.regWrite(addr, data)) {
        return true;
    }
    if (ch3.regWrite(addr, data)) {
        return true;
    }
    if (ch4.regWrite(addr, data)) {
        return true;
    }

    switch(addr) {
        case NR50: nr50 = data; break;
        case NR51: nr51 = data; break;
        case NR52: nr52 = data; break;
        default:   return false;
    }
    
    return true;
}

bool APU::regRead(uint16_t addr, uint8_t &val) {
    if (ch1.regRead(addr, val)) {
        return true;
    }
    if (ch2.regRead(addr, val)) {
        return true;
    }
    if (ch3.regRead(addr, val)) {
        return true;
    }
    if (ch4.regRead(addr, val)) {
        return true;
    }

    switch(addr) {
        case NR50: val = nr50; break;
        case NR51: val = nr51; break;
        case NR52: 
            val = (nr52 & 0xF0) |
                  (ch1.isEnabled() << 0) |
                  (ch2.isEnabled() << 1) |
                  (ch3.isEnabled() << 2) |
                  (ch4.isEnabled() << 3); 
                  
            break;
        default:   return false;
    }

    return true;
}
