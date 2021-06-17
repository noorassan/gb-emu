#pragma once

#include <cstdint>
#include <array>

#include "apu/channel.h"


class Channel4 : public Channel {
    public:
        Channel4();
        ~Channel4() = default;

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
		uint8_t nr41;
		uint8_t nr42;
		uint8_t nr43;
		uint8_t nr44;

        bool enabled;
};