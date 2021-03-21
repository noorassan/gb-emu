#include <thread>

#include "bus.h"
#include "controls.h"
#include "sdl_gb_driver.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

SDLGameboyDriver::SDLGameboyDriver(std::string title) {
    SDL_Init(SDL_INIT_VIDEO);
    window = nullptr;
    renderer = nullptr;

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH * 2,
                                SCREEN_HEIGHT * 2,
                                SDL_WINDOW_OPENGL,
                                &window,
                                &renderer);

    SDL_SetWindowTitle(window, title.c_str());

    time = std::chrono::steady_clock::now();
}

SDLGameboyDriver::~SDLGameboyDriver() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void SDLGameboyDriver::setRendererDrawColor(COLOR color) {
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
        case UNLIT:
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
}

void SDLGameboyDriver::draw(COLOR color, uint8_t x, uint8_t y) {
    if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
        setRendererDrawColor(color);
        SDL_RenderDrawPoint(renderer, (2 * x), (2 * y));
        SDL_RenderDrawPoint(renderer, (2 * x) + 1, (2 * y));
        SDL_RenderDrawPoint(renderer, (2 * x), (2 * y) + 1);
        SDL_RenderDrawPoint(renderer, (2 * x) + 1, (2 * y) + 1);
    }
}

void SDLGameboyDriver::render() {
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
    
    std::this_thread::sleep_until(time + std::chrono::milliseconds(17));
    time = std::chrono::steady_clock::now();
}

CONTROL SDLGameboyDriver::pollControls() {
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

    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(argv[1]);
    SDLGameboyDriver driver = SDLGameboyDriver(cart->getTitle());

    Bus bus(&driver);
    bus.insertCartridge(cart);
    bus.reset();

    bus.run();
    return EXIT_SUCCESS;
}