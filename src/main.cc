#include <chrono>
#include <memory>
#include <thread>
#include <iostream>

#include <SDL.h>
#include "bus.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144


bool startSDL(SDL_Window **window_ptr, SDL_Surface **surface_ptr, std::string title) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    *window_ptr = SDL_CreateWindow(title.c_str(),
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED, 
                                   SCREEN_WIDTH, 
                                   SCREEN_HEIGHT,
                                   SDL_WINDOW_OPENGL);
    
    if (*window_ptr == NULL) {
        return false;
    }
    
    *surface_ptr = SDL_GetWindowSurface(*window_ptr);
    return true;
}

int main(int argc, char **argv) {
    //std::chrono::high_resolution_clock::time_point t1, t2;
    //std::chrono::duration<float, std::milli> time_span;
    Bus bus;
    SDL_Window *window;
    SDL_Surface *surface;

    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(argv[1]);
    bus.insertCartridge(cart);
    bus.reset(); 

    if (!startSDL(&window, &surface, cart->getTitle())) {
        throw std::runtime_error("Failed to start and SDL window.");
    }

    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xF0, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);

    SDL_Event event;
    bool quit = false;

    while(!quit) {
        //t1 = std::chrono::high_resolution_clock::now();

        // Gameboy clock runs at 4.19 mHz.
        // For convenience, we do all the clocks in a one second interval at once
        // and then wait for the remainder of the second.
        // 4.19 mHz / 60 Hz = 69,833.33 -> 69,833
        for(uint32_t i = 0; i < 69833; i++) {
            if (i % 1133 == 0) {
                SDL_PollEvent(&event);
                quit = event.type == SDL_QUIT;
            }

            bus.clock();
        }

        //t2 = std::chrono::high_resolution_clock::now();
        //time_span = std::chrono::seconds(1) - (t2 - t1);
        //std::this_thread::sleep_for(time_span);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
