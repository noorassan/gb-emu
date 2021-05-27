#include "bus.h"
#include "sdl_gb_driver.h"

SDLGameboyDriver::SDLGameboyDriver(std::string title) : GameboyDriver() {
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

    int32_t num_keys;
    keyboard_state = SDL_GetKeyboardState(&num_keys);

    // Lock texture so we're ready to start drawing pixels to it
    int32_t pitch;
    SDL_LockTexture(texture, nullptr, (void **) &pixels, &pitch);

    time = std::chrono::steady_clock::now();
    quit = false;
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

bool SDLGameboyDriver::quitReceived() {
    return quit;
}

ControllerState SDLGameboyDriver::pollControls() {
    ControllerState controls;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
    }

    controls.a      = keyboard_state[SDL_SCANCODE_X];
    controls.b      = keyboard_state[SDL_SCANCODE_Z];
    controls.select = keyboard_state[SDL_SCANCODE_BACKSPACE];
    controls.start  = keyboard_state[SDL_SCANCODE_RETURN];
    controls.right  = keyboard_state[SDL_SCANCODE_RIGHT];
    controls.left   = keyboard_state[SDL_SCANCODE_LEFT];
    controls.up     = keyboard_state[SDL_SCANCODE_UP];
    controls.down   = keyboard_state[SDL_SCANCODE_DOWN];

    return controls;
}


int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " rom_file" << std::endl;
        return EXIT_FAILURE;
    }

    std::string gb_filename = std::string(argv[1]);
    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(gb_filename);
    SDLGameboyDriver driver = SDLGameboyDriver(cart->getTitle());

    Bus bus(&driver);
    bus.insertCartridge(cart);
    bus.reset();

    // load save file if it exists
    std::string save_filename = gb_filename.substr(0, gb_filename.find_last_of(".")) + ".sav";
    std::ifstream save_file = std::ifstream(save_filename);
    if (save_file.good()) {
        bus.loadState(save_filename);
    }
    save_file.close();

    bus.run();

    // save state before exiting
    bus.saveState(save_filename);
    return EXIT_SUCCESS;
}