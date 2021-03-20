#pragma once

#include <cstdint>

#include "color.h"
#include "controls.h"

class GameboyDriver {
    public:
        virtual ~GameboyDriver() = default;

    public:
        // Draw a pixel of specified color to the screen at location (x, y)
        virtual void draw(COLOR color, uint8_t x, uint8_t y) = 0;

        // Render the screen and wait for the rest of the frame
        virtual void render() = 0;

        // Get control inputs
        virtual CONTROL pollControls() = 0;
};