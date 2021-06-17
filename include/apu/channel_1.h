#pragma once

#include <cstdint>
#include <array>

#include "apu/channel.h"
#include "apu/length_counter.h"


class Channel1 : public LengthChannel {
    public:
        Channel1();
        ~Channel1() = default;

    public:
        bool regWrite(uint16_t addr, uint8_t data);
        bool regRead(uint16_t addr, uint8_t &val);

        void reset() override;
        void clock(uint8_t clocks) override;
        uint8_t getOutput() override;

        bool isEnabled() override;
        void setEnabled(bool enabled) override;

        uint16_t getLength() override;
        void setLength(uint16_t length) override;

        bool isLengthEnabled() override;
        void setLengthEnabled(bool enabled) override;

    private:
        LengthCounter len_counter;

    private:
        void trigger() override;

        uint16_t getFrequency();
        uint8_t getVolume();
        uint8_t getDuty();

    private:
        std::array<uint8_t, 4> duty_cycles;

        uint8_t nr10;
        uint8_t nr11;
        uint8_t nr12;
        uint8_t nr13;
        uint8_t nr14;

        uint8_t duty_pointer;
        uint16_t duty_timer;

        bool enabled;
};