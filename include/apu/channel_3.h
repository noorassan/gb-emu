#pragma once

#include <cstdint>
#include <array>

#include "apu/channel.h"
#include "apu/length_counter.h"


class Channel3 : public LengthChannel {
    public:
        Channel3();
        ~Channel3() = default;

    public:
        bool regWrite(uint16_t addr, uint8_t data);
        bool regRead(uint16_t addr, uint8_t &val);

        void reset() override;
        void clock(uint8_t clocks) override;
        uint8_t getOutput() override;

        bool isEnabled() override;
        void setEnabled(bool enabled) override;

        uint8_t getLength() override;
        void setLength(uint8_t length) override;

        bool isLengthEnabled() override;
        void setLengthEnabled(bool enabled) override;

    private:
        LengthCounter len_counter;

    private:
        void trigger() override;

        uint16_t getFrequency();
        uint8_t getVolumeCode();

    private:
        uint8_t nr30;
        uint8_t nr31;
        uint8_t nr32;
        uint8_t nr33;
        uint8_t nr34;

        std::array<uint8_t, 0x10> wave_pattern_ram;

        uint16_t table_timer;
        uint8_t table_pointer;
        uint8_t sample;

        bool enabled;
};