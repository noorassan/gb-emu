#pragma once

#include <chrono>
#include <iostream>
#include <thread>

#include <SDL.h>

#include "gb_driver.h"

#define SAMPLE_SIZE 2048
#define SAMPLE_RATE 48000
#define SCALE_FACTOR 3

class SDLGameboyDriver : public GameboyDriver {
    public:
        SDLGameboyDriver(std::string title);
        ~SDLGameboyDriver();

    public:
        // Draw a pixel of specified color to the screen at location (x, y)
        void draw(COLOR color, uint8_t x, uint8_t y) override;

        // Render the screen and wait for the rest of the frame
        void render() override;

        // Push an audio sample
        void pushSample(AudioOutput output) override;

        // Return true if a QUIT input has been received
        bool quitReceived() override;

        // Returns a ControllerState representing currently pressed controls
        ControllerState pollControls() override;
    
    private:
        uint32_t getARGBColor(COLOR color);

    private:
        SDL_Renderer *renderer;
        SDL_Window *window;
        SDL_Texture *texture;
        SDL_Event event;

        const uint8_t *keyboard_state;
        uint32_t *pixels;

        uint8_t audio_device_id;
        uint32_t samples_stored;
        std::array<float, SAMPLE_SIZE> samples;

        std::chrono::steady_clock::time_point time;
};