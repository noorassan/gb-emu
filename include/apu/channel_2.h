#pragma once

#include <cstdint>
#include <array>

#include "apu/channel.h"
#include "apu/length_counter.h"
#include "apu/volume_envelope.h"


class Channel2 : public LengthChannel, public EnvelopeChannel {
    public:
        Channel2();
        ~Channel2() = default;

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

        uint16_t getFrequency();
        uint8_t getDuty();

    private:
        std::array<uint8_t, 4> duty_cycles;

        uint8_t nr21;
        uint8_t nr22;
        uint8_t nr23;
        uint8_t nr24;

        uint8_t duty_pointer;
        uint16_t duty_timer;

        bool enabled;
};