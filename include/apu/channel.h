#pragma once

#include <cstdint>

#define FREQ_TO_PERIOD(x) (2048 - x) * 4


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
        virtual uint16_t getLength() = 0;
        virtual void setLength(uint16_t length) = 0;

        virtual bool isLengthEnabled() = 0;
        virtual void setLengthEnabled(bool enabled) = 0;
};


class FrequencyChannel : public Channel {
    public:
        virtual ~FrequencyChannel() = default;

    public:
};