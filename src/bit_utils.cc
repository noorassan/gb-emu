#include "bit_utils.h"

uint16_t highestOrderBit(uint16_t num) {
    if (!num) return 0;

    uint16_t ret = 1;

    while (num >>= 1) {
        ret <<= 1;
    }

    return ret;
}