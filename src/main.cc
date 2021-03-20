#include <chrono>
#include <memory>
#include <thread>
#include <iostream>

#include <SDL.h>

#include "bus.h"
#include "controls.h"

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

    SDL_SetWindowTitle(window, title.c_str());
    
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
        SDL_RenderDrawPoint(renderer, (2 * x), (2 * y));
        SDL_RenderDrawPoint(renderer, (2 * x) + 1, (2 * y));
        SDL_RenderDrawPoint(renderer, (2 * x), (2 * y) + 1);
        SDL_RenderDrawPoint(renderer, (2 * x) + 1, (2 * y) + 1);
    }
}

CONTROL pollControls() {
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT) {
        return QUIT;
    }

    return NONE;
}


int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " rom_file" << std::endl;
        return EXIT_FAILURE;
    }

    std::chrono::steady_clock::time_point time;
    Bus bus(drawPixel, pollControls);

    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(argv[1]);
    bus.insertCartridge(cart);
    bus.reset(); 

    if (!startSDL(cart->getTitle())) {
        throw std::runtime_error("Failed to start SDL.");
    }

    bool quit = false;
    while(!quit) {
        time = std::chrono::steady_clock::now();
        SDL_RenderClear(renderer);

        // 70224 clocks per frame
        quit = bus.clock(70224);

        SDL_RenderPresent(renderer);
        std::this_thread::sleep_until(time + std::chrono::milliseconds(17));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
