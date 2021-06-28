#include "bus.h"
#include "sdl_gb_driver.h"


SDLGameboyDriver::SDLGameboyDriver(std::string title) : GameboyDriver(SAMPLE_RATE) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    window = SDL_CreateWindow(title.c_str(), 
                     SDL_WINDOWPOS_UNDEFINED,
                     SDL_WINDOWPOS_UNDEFINED,
                     SCREEN_WIDTH * SCALE_FACTOR,
                     SCREEN_HEIGHT * SCALE_FACTOR,
                     SDL_WINDOW_OPENGL);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                SCREEN_WIDTH * SCALE_FACTOR,
                                SCREEN_HEIGHT * SCALE_FACTOR);

    int32_t num_keys;
    keyboard_state = SDL_GetKeyboardState(&num_keys);

    // Lock texture so we're ready to start drawing pixels to it
    int32_t pitch;
    SDL_LockTexture(texture, nullptr, (void **) &pixels, &pitch);

    SDL_AudioSpec audio_settings;
    audio_settings.freq = sampling_rate;
    audio_settings.format = AUDIO_F32SYS;
    audio_settings.channels = 2;
    audio_settings.callback = nullptr;
    samples_stored = 0;

    const char *audio_device_name = SDL_GetAudioDeviceName(0, 0);;
    audio_device_id = SDL_OpenAudioDevice(audio_device_name, false, &audio_settings, nullptr, false);
    SDL_PauseAudioDevice(audio_device_id, false);
    std::cout << audio_device_name << std::endl;

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
    if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
        uint32_t argb_color = getARGBColor(color);
        for (uint8_t y_disp = 0; y_disp < SCALE_FACTOR; y_disp++) {
            for (uint8_t x_disp = 0; x_disp < SCALE_FACTOR; x_disp++) {
                uint32_t i = ((x * SCALE_FACTOR) + x_disp) + ((y * SCALE_FACTOR) + y_disp) * (SCREEN_WIDTH * SCALE_FACTOR);
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
    
    // don't sleep if we don't have enough audio samples
    if (SDL_GetQueuedAudioSize(audio_device_id) > sampling_rate) {
        std::this_thread::sleep_until(time + std::chrono::nanoseconds(16742706));
    }
    
    time = std::chrono::steady_clock::now();
}

void SDLGameboyDriver::pushSample(AudioOutput output) {
    // samples are only provided at the rate that we request in sampling_rate, so we don't need to downsample
    float unprocessed_sample;
    float left_sample = 0;
    float right_sample = 0;

    // mix channel 1
    unprocessed_sample = output.ch1_left / 100.0F;
    SDL_MixAudioFormat((uint8_t *) &left_sample, (uint8_t *) &unprocessed_sample, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME / 100);
    unprocessed_sample = output.ch1_right / 100.0F;
    SDL_MixAudioFormat((uint8_t *) &right_sample, (uint8_t *) &unprocessed_sample, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME / 100);

    // mix channel 2
    unprocessed_sample = output.ch2_left / 100.0F;
    SDL_MixAudioFormat((uint8_t *) &left_sample, (uint8_t *) &unprocessed_sample, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME / 100);
    unprocessed_sample = output.ch2_right / 100.0F;
    SDL_MixAudioFormat((uint8_t *) &right_sample, (uint8_t *) &unprocessed_sample, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME / 100);

    // mix channel 3 
    unprocessed_sample = output.ch3_left / 100.0F;
    SDL_MixAudioFormat((uint8_t *) &left_sample, (uint8_t *) &unprocessed_sample, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME / 100);
    unprocessed_sample = output.ch3_right / 100.0F;
    SDL_MixAudioFormat((uint8_t *) &right_sample, (uint8_t *) &unprocessed_sample, AUDIO_F32SYS, sizeof(float), SDL_MIX_MAXVOLUME / 100);

    // push left and right samples
    samples[samples_stored] = left_sample;
    samples[samples_stored + 1] = right_sample;
    samples_stored += 2;

    if (samples_stored >= SAMPLE_SIZE) {
        SDL_QueueAudio(audio_device_id, samples.data(), SAMPLE_SIZE * sizeof(float));
        samples_stored = 0;
    }
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
    std::string save_filename = gb_filename.substr(0, gb_filename.find_last_of(".")) + ".sav";

    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>(gb_filename);
    SDLGameboyDriver driver = SDLGameboyDriver(cart->getTitle());

    Bus bus(&driver);
    bus.insertCartridge(cart);
    bus.reset();

    // load save file if it exists
    bus.loadState(save_filename);

    bus.run();

    // save state before exiting
    bus.saveState(save_filename);
    
    return EXIT_SUCCESS;
}
