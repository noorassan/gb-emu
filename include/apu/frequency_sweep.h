#pragma once

#include <cstdint>

#define FREQ_CLOCKS 32768 

class FrequencyChannel;


class FrequencySweep {
    public:
        FrequencySweep(FrequencyChannel *ch);
        ~FrequencySweep() = default;

    public:
        void reset();
        void clock(uint8_t clocks);
        void trigger();

    private:
        uint16_t recalculateFrequency();

    private:
        FrequencyChannel *ch;

        uint16_t shadow_register;
        uint16_t timer;
        bool enabled;
};