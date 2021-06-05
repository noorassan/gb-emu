#include "apu.h"


bool APU::isCh3Enabled() {
    return nr52 & 0x04;
}

uint16_t APU::getCh3Frequency() {
    return nr33 + ((nr34 & 0x07) << 8);
}
