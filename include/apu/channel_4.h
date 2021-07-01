#pragma once

#include <cstdint>
#include <array>

#include "apu/channel.h"
#include "apu/length_counter.h"
#include "apu/volume_envelope.h"


class Channel4 : public LengthChannel, public EnvelopeChannel {
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

        uint8_t getLength() override;
        void setLength(uint8_t length) override;

        bool isLengthEnabled() override;
        void setLengthEnabled(bool enabled) override;

        uint8_t getVolume() override;

        uint8_t getEnvelopePeriod() override;
        bool isAddModeEnabled() override;

    private:
        LengthCounter len_counter;
        VolumeEnvelope envelope;

    private:
        void trigger() override;

        uint8_t getClockShift();
        uint8_t getDivisor();
        bool isWidthModeEnabled();

	private:
        std::array<uint8_t, 8> divisors;

		uint8_t nr41;
		uint8_t nr42;
		uint8_t nr43;
		uint8_t nr44;

        uint16_t timer;
        uint16_t lfsr;   // Linear feedback shift register

        bool enabled;
};