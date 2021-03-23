#pragma once

#include <chrono>
#include <iostream>
#include <thread>

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

        // Return true if a QUIT input has been received
        bool quitReceived() override;

        // Get control inputs
        // Returns a union of CONTROLs
        uint8_t pollControls(uint8_t p1) override;
    
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