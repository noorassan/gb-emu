#pragma once

#include <cstdint>

#include "color.h"
#include "controller_state.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144


class GameboyDriver {
    public:
        GameboyDriver() = default;
        virtual ~GameboyDriver() = default;

    public:
        // Draw a pixel of specified color to the screen at location (x, y)
        virtual void draw(COLOR color, uint8_t x, uint8_t y) = 0;

        // Render the screen and wait for the rest of the frame
        virtual void render() = 0;

        // Return true if a QUIT input has been received
        virtual bool quitReceived() = 0;

        // Returns a ControllerState representing currently pressed controls
        virtual ControllerState pollControls() = 0;

    protected:
        bool quit;
};