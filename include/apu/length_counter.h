#pragma once

#include <cstdint>

#define LEN_CLOCKS 16384

class LengthChannel;


class LengthCounter {
    public:
        LengthCounter(LengthChannel *ch);
        ~LengthCounter() = default;

    public:
        void reset();
        void clock(uint8_t clocks);

    private:
        LengthChannel *ch;

        uint16_t length_timer;

};