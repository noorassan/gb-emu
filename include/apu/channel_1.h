#pragma once

#include <cstdint>
#include <array>

#include "apu/channel.h"
#include "apu/length_counter.h"
#include "apu/frequency_sweep.h"
#include "apu/volume_envelope.h"


class Channel1 : public LengthChannel, public SweepChannel, public EnvelopeChannel {
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

        uint8_t getLength() override;
        void setLength(uint8_t length) override;

        bool isLengthEnabled() override;
        void setLengthEnabled(bool enabled) override;

        uint16_t getFrequency() override;
        void setFrequency(uint16_t frequency) override;

        uint8_t getSweepShift() override;
        uint8_t getSweepPeriod() override;
        bool getSweepNegate() override;

        uint8_t getVolume() override;

        uint8_t getEnvelopePeriod() override;
        bool isAddModeEnabled() override;

    private:
        LengthCounter len_counter;
        FrequencySweep freq_sweep;
        VolumeEnvelope envelope;

    private:
        void trigger() override;

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