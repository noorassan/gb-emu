#include "apu/frequency_sweep.h"
#include "apu/channel.h"

FrequencySweep::FrequencySweep(FrequencyChannel *ch) {
    this->ch = ch;
    reset();
}

void FrequencySweep::reset() {
    shadow_register = ch->getFrequency();
    timer = FREQ_CLOCKS;
    enabled = true;
}

void FrequencySweep::clock(uint8_t clocks) {
    if (clocks >= timer) {
        uint8_t sweep_period = ch->getSweepPeriod();
        timer = (FREQ_CLOCKS * sweep_period) - (clocks - timer);
        
        if (sweep_period && enabled) {
            uint16_t new_frequency = recalculateFrequency();
            if (new_frequency >= 0x800) {
                ch->setEnabled(false);
            } else if (ch->getSweepShift()) {
                ch->setFrequency(new_frequency);
                shadow_register = new_frequency;
                new_frequency = recalculateFrequency();
                if (new_frequency >= 0x800) ch->setEnabled(false);
            }
        }
    } else {
        timer -= clocks;
    }
}

void FrequencySweep::trigger() {
    shadow_register = ch->getFrequency();
    timer = FREQ_CLOCKS;
    enabled = ch->getSweepPeriod() || ch->getSweepShift();

    if (ch->getSweepShift()) {
        uint16_t new_frequency = recalculateFrequency();
        if (new_frequency >= 0x800) ch->setEnabled(false);
    }
}

uint16_t FrequencySweep::recalculateFrequency() {
    if (ch->getSweepNegate()) {
        return shadow_register - (shadow_register >> ch->getSweepShift());
    } else {
        return shadow_register + (shadow_register >> ch->getSweepShift());
    }
}