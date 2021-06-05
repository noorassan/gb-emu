#include "apu.h"


bool APU::isCh4Enabled() {
    return nr52 & 0x08;
}