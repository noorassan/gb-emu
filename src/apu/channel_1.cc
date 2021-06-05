#include "apu.h"


void APU::clockCh1(uint8_t clocks) {
    if (ch1_timer == 0) {
        ch1_timer = FREQ_TO_PERIOD(getCh1Frequency());
        ch1_pointer = (ch1_pointer + 1) % 8;
    }

    ch1_timer--;
}

void APU:: resetCh1() {
    ch1_pointer = 0;
    ch1_timer = FREQ_TO_PERIOD(getCh1Frequency());
    ch1_freq_timer = FREQ_CLOCKS; 
}

bool APU::isCh1Enabled() {
    return nr52 & 0x01;
}

uint8_t APU::getCh1Output() {
    return ((getCh1Duty() >> ch1_pointer) & 0x01) * getCh1Volume();
}

uint8_t APU::getCh1Duty() {
    return duty_cycles[(nr11 & 0xC0) >> 6];
}

uint8_t APU::getCh1Volume() {
    return (nr12 & 0xF0) >> 4;
}

uint16_t APU::getCh1Frequency() {
    return nr13 + ((nr14 & 0x07) << 8);
}
