#include "apu/length_counter.h"
#include "apu/channel.h"

LengthCounter::LengthCounter(LengthChannel *ch) {
    this->ch = ch;
    reset();
}

void LengthCounter::reset() {
    length_timer = LEN_CLOCKS;
}

void LengthCounter::clock(uint8_t clocks) {
    if (clocks >= length_timer) {
        length_timer = LEN_CLOCKS - (clocks - length_timer);
        uint8_t len = ch->getLength();

        // if length is disabled, there's nothing to do
        if (ch->isLengthEnabled() && len) {
            len--;
            ch->setLength(len);

            if (len == 0) {
                ch->setEnabled(false);
            }
        }
    } else {
        length_timer -= clocks;
    }
}

void LengthCounter::trigger() {
    if (!ch->getLength()) {
        ch->setLength(0xFF);
    }
}