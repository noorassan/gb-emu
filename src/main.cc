#include <thread>

#include "bus.h"
#include "controls.h"
#include "sdl_gb_driver.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

SDLGameboyDriver::SDLGameboyDriver(std::string title) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(title.c_str(), 
                     SDL_WINDOWPOS_UNDEFINED,
                     SDL_WINDOWPOS_UNDEFINED,
                     SCREEN_WIDTH * 2,
                     SCREEN_HEIGHT * 2,
                     SDL_WINDOW_OPENGL);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                SCREEN_WIDTH * 2,
                                SCREEN_HEIGHT * 2);

    // Lock texture so we're ready to start drawing pixels to it
    int pitch;
    SDL_LockTexture(texture, nullptr, (void **) &pixels, &pitch);

    time = std::chrono::steady_clock::now();
}

SDLGameboyDriver::~SDLGameboyDriver() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

uint32_t SDLGameboyDriver::getARGBColor(COLOR color) {
    uint8_t r, g, b;
    switch (color) {
        case WHITE:
            r = 155;
            g = 188;
            b = 15;
            break;
        case LIGHT_GREEN:
            r = 139;
            g = 172;
            b = 15;
            break;
        case DARK_GREEN:
            r = 48;
            g = 98;
            b = 48;
            break;
        case BLACK:
            r = 15;
            g = 56;
            b = 15;
            break;
        case UNLIT:
            r = g = b = 255;
            break;
    }

    return (r << 16) | (g << 8) | (b << 0);
}

void SDLGameboyDriver::draw(COLOR color, uint8_t x, uint8_t y) {
    uint8_t scale_factor = 2;
    if (y > SCREEN_HEIGHT / 2 + 1) {
        ;
    }
    if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
        uint32_t argb_color = getARGBColor(color);
        for (uint8_t y_disp = 0; y_disp < scale_factor; y_disp++) {
            for (uint8_t x_disp = 0; x_disp < scale_factor; x_disp++) {
                uint32_t i = ((x * scale_factor) + x_disp) + ((y * scale_factor) + y_disp) * (SCREEN_WIDTH * scale_factor);
                pixels[i] = argb_color;
            }
        }
    }
}

void SDLGameboyDriver::render() {
    SDL_UnlockTexture(texture);

    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);

    int pitch;
    SDL_LockTexture(texture, nullptr, (void **) &pixels, &pitch);
    
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