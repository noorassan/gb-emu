#pragma once

#include <cstdint>

#define SQUARE_FREQ_TO_PERIOD(x) (2048 - x) * 4
#define WAVE_FREQ_TO_PERIOD(x) (2048 - x) * 2


class Channel {
    public:
        virtual ~Channel() = default;

    public:
        virtual void reset() = 0;
        virtual void clock(uint8_t clocks) = 0;
        virtual uint8_t getOutput() = 0;
        
        virtual bool isEnabled() = 0;
        virtual void setEnabled(bool enabled) = 0;

    private:
        virtual void trigger() = 0;
};


class LengthChannel : public Channel {
    public:
        virtual ~LengthChannel() = default;

    public:
        virtual uint8_t getLength() = 0;
        virtual void setLength(uint8_t length) = 0;

        virtual bool isLengthEnabled() = 0;
        virtual void setLengthEnabled(bool enabled) = 0;
};


class SweepChannel : public Channel {
    public:
        virtual ~SweepChannel() = default;

    public:
        virtual uint16_t getFrequency() = 0;
        virtual void setFrequency(uint16_t frequency) = 0;

        virtual uint8_t getSweepShift() = 0;
        virtual uint8_t getSweepPeriod() = 0;
        virtual bool getSweepNegate() = 0;
};

class EnvelopeChannel : public Channel {
    public:
        virtual ~EnvelopeChannel() = default;

    public:
        virtual uint8_t getVolume() = 0;

        virtual uint8_t getEnvelopePeriod() = 0;
        virtual bool isAddModeEnabled() = 0;
};