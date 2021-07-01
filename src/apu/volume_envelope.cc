#include "apu/volume_envelope.h"
#include "apu/channel.h"

VolumeEnvelope::VolumeEnvelope(EnvelopeChannel *ch) {
    this->ch = ch;
    reset();
}

void VolumeEnvelope::reset() {
    timer = ENVELOPE_CLOCKS; 
    enabled = true;
    internal_volume = ch->getVolume();
}

void VolumeEnvelope::clock(uint8_t clocks) {
    if (clocks >= timer) {
        uint8_t envelope_period = ch->getEnvelopePeriod();
        timer = (ENVELOPE_CLOCKS * envelope_period) - (clocks - timer);

        if (ch->getEnvelopePeriod()) {
            if (ch->isAddModeEnabled() && internal_volume <= 0x0E) {
                internal_volume++;
            } else if (!ch->isAddModeEnabled() && internal_volume >= 0x01) {
                internal_volume--;
            } else {
                enabled = false;
            }
        }
    } else {
        timer -= clocks;
    }
}

void VolumeEnvelope::trigger() {
    timer = ENVELOPE_CLOCKS;
    enabled = true;
    internal_volume = ch->getVolume();
}

uint8_t VolumeEnvelope::getVolume() {
    return internal_volume;
}