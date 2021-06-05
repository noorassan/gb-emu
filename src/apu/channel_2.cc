#include "apu.h"


void APU::clockCh2(uint8_t clocks) {
    uint32_t period = FREQ_TO_PERIOD(getCh2Frequency());

    if (clocks >= ch2_timer) {
        ch2_pointer += 1 + ((clocks - ch2_timer) / period);
        ch2_pointer %= 8;

        ch2_timer = period - ((clocks - ch2_timer) % period);
    } else {
        ch2_timer -= clocks;
    }
}

void APU::resetCh2() {
    ch2_pointer = 0;
    ch2_timer = FREQ_TO_PERIOD(getCh2Frequency());
}

bool APU::isCh2Enabled() {
    return nr52 & 0x02;
}

uint8_t APU::getCh2Duty() {
    return duty_cycles[(nr21 & 0xC0) >> 6];
}

uint8_t APU::getCh2Volume() {
    return (nr22 & 0xF0) >> 4;
}

uint16_t APU::getCh2Frequency() {
    return nr23 + ((nr24 & 0x07) << 8);
}