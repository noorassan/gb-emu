#pragma once

#include <cstdint>

#include "audio_output.h"
#include "color.h"
#include "controller_state.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define GB_CLOCK_RATE 4194304


class GameboyDriver {
    public:
        GameboyDriver(uint32_t sampling_rate) : sampling_rate(sampling_rate) {}
        virtual ~GameboyDriver() = default;

    public:
        // Draw a pixel of specified color to the screen at location (x, y)
        virtual void draw(COLOR color, uint8_t x, uint8_t y) = 0;

        // Render the screen and wait for the rest of the frame
        virtual void render() = 0;

        // Push an audio sample
        virtual void pushSample(AudioOutput output) = 0;

        // Return true if a QUIT input has been received
        virtual bool quitReceived() = 0;

        // Returns a ControllerState representing currently pressed controls
        virtual ControllerState pollControls() = 0;

    public:
        const uint32_t sampling_rate;

    protected:
        bool quit;
};