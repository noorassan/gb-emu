#include <chrono>
#include <memory>
#include <thread>
#include <iostream>

#include <SDL.h>
#include "bus.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Event event;

bool startSDL(std::string title) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }
    window = NULL;
    renderer = NULL;

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH * 2,
                                SCREEN_HEIGHT * 2,
                                SDL_WINDOW_OPENGL,
                                &window,
                                &renderer);
    
    if (window == NULL || renderer == NULL) {
        return false;
    }
    
    return true;
}

void setRendererDrawColor(COLOR color) {
    switch (color) {
        case WHITE:
            SDL_SetRenderDrawColor(renderer, 155, 188, 15, 255);
            break;
        case LIGHT_GREEN:
            SDL_SetRenderDrawColor(renderer, 139, 172, 15, 255);
            break;
        case DARK_GREEN:
            SDL_SetRenderDrawColor(renderer, 48, 98, 48, 255);
            break;
        case BLACK:
            SDL_SetRenderDrawColor(renderer, 15, 56, 15, 255);
            break;
    }
}

void drawPixel(COLOR color, uint8_t x, uint8_t y) {
    if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
        setRendererDrawColor(color);
        SDL_RenderDrawPoint(renderer, 2 * x, 2 * y);
        SDL_RenderDrawPoint(renderer, 2 * x + 1, 2 * y);
        SDL_RenderDrawPoint(renderer, 2 * x, 2 * y + 1);
        SDL_RenderDrawPoint(renderer, 2 * x + 1, 2 * y + 1);
    }
}


int main(int argc, char **argv) {
    std::chrono::high_resolution_clock::time_point t1, t2;
    std::chrono::duration<float, std::milli> time_span;
    Bus bus(drawPixel);

    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(argv[1]);
    bus.insertCartridge(cart);
    bus.reset(); 

    if (!startSDL(cart->getTitle())) {
        throw std::runtime_error("Failed to start and SDL window.");
    }

    bool quit = false;
    while(!quit) {
        t1 = std::chrono::high_resolution_clock::now();

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Gameboy clock runs at 4.19 mHz.
        // For convenience, we do all the clocks in a one second interval at once
        // and then wait for the remainder of the second.
        // 4.19 mHz / 60 Hz = 69,833.33 -> 69,833
        for(uint32_t i = 0; i < 70224; i++) {
            if (i % 1000 == 0) {
                SDL_PollEvent(&event);
                quit = event.type == SDL_QUIT;
            }

            bus.clock();
        }

        t2 = std::chrono::high_resolution_clock::now();
        time_span = std::chrono::milliseconds(17) - (t2 - t1);
        std::this_thread::sleep_for(time_span);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
