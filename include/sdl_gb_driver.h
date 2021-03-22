#pragma once

#include <iostream>
#include <chrono>

#include <SDL.h>

#include "gb_driver.h"


class SDLGameboyDriver : public GameboyDriver {
    public:
        SDLGameboyDriver(std::string title);
        ~SDLGameboyDriver();

    public:
        // Draw a pixel of specified color to the screen at location (x, y)
        void draw(COLOR color, uint8_t x, uint8_t y) override;

        // Render the screen and wait for the rest of the frame
        void render() override;

        // Get control inputs
        CONTROL pollControls() override;
    
    private:
        uint32_t getARGBColor(COLOR color);

    private:
        SDL_Renderer *renderer;
        SDL_Window *window;
        SDL_Texture *texture;
        SDL_Event event;

        uint32_t *pixels;

        std::chrono::steady_clock::time_point time;
};