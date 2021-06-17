#pragma once

#include <cstdint>
#include <array>

#include "apu/channel.h"


class Channel3 : public Channel {
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

    private:
        void trigger() override;

    private:
        uint8_t nr30;
        uint8_t nr31;
        uint8_t nr32;
        uint8_t nr33;
        uint8_t nr34;

        std::array<uint8_t, 0x10> wave_pattern_ram;

        bool enabled;
};