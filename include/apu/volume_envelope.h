#pragma once

#include <cstdint>

#define ENVELOPE_CLOCKS 65536

class EnvelopeChannel;


class VolumeEnvelope {
    public:
        VolumeEnvelope(EnvelopeChannel *ch);
        ~VolumeEnvelope() = default;

    public:
        void reset();
        void clock(uint8_t clocks);
        void trigger();
        uint8_t getVolume();

    private:
        EnvelopeChannel *ch;

        uint8_t internal_volume;
        uint32_t timer;
        bool enabled;
};