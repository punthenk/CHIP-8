#include "Chip8.h"
#include <iostream>
#include <SDL2/SDL.h>
#include <thread>

#include "Audio.h"

Chip8 chip8;
Audio audio;

uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int const WIN_WIDTH = 1024;
int const WIN_HEIGHT = 512;

int main(int argc, char *argv[]) {
    char* rom_file_path;

    if (argc > 1) {
        rom_file_path = argv[1];
    } else {
        std::cerr << "Give the ROM file path as parameter" << std::endl;
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL2 could not init" << std::endl;
        return 1;
    }

    // Create SDL window
    SDL_Window* p_window = SDL_CreateWindow(
        "CHIP8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH,
        WIN_HEIGHT,
        SDL_WINDOW_OPENGL
    );

    if (!p_window) {
        std::cerr << "Failed to CreateWindow: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Set window to front
    SDL_RaiseWindow(p_window);

    // Create renderer
    SDL_Renderer* p_renderer = SDL_CreateRenderer(p_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!p_renderer) {
        std::cerr << "Failed to CreateRenderer: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_RenderSetLogicalSize(p_renderer, WIN_WIDTH, WIN_HEIGHT);

    // Create texture
    SDL_Texture* p_texture = SDL_CreateTexture(
        p_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        64,
        32);

    if (!p_texture) {
        std::cerr << "Failed to CreateTexture: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL audio
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL2 could not initialize audio: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_AudioSpec want;
    SDL_AudioSpec have;

    SDL_zero(want);
    want.freq = audio.SAMPLE_RATE;
    want.format = AUDIO_S16SYS; // 16-bit getekende integers (system byte order)
    want.channels = 1;          // Mono geluid is voldoende voor CHIP-8
    want.samples = 2048;        // Buffergrootte (lage latency)
    want.callback = Audio::audio_callback;
    want.userdata = &audio;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (device == 0) {
        std::cerr << "Could not open audio device: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Temporary pixel buffer
    uint32_t pixels[2048];
    uint8_t sound_timer = 0;

    if (!chip8.loadROM(rom_file_path)) {
        std::cerr << "Failed to load the ROM" << std::endl;
        return 1;
    }

    for (;;) {
        chip8.emulateCycle();
        // printf("%d", chip8.drawFlag);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                exit(0);

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);

                for (int i = 0; i < 16; i++) {
                    if (event.key.keysym.sym == keymap[i]) {
                        chip8.key[i] = 1;
                    }
                }
            }

            if (event.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (event.key.keysym.sym == keymap[i]) {
                        chip8.key[i] = 0;
                    }
                }
            }
        }

        if (chip8.sound_timer > 0) {
            SDL_PauseAudioDevice(device, 0);
            SDL_Delay(16);
        }

        SDL_PauseAudioDevice(device, 1);

        if (chip8.drawFlag) {
            chip8.drawFlag = false;

            // Store pixels in temporary buffer
            for (int i = 0; i < 2048; i++) {
                uint8_t pixel = chip8.gfx[i];
                pixels[i] = (0xFFFFFFFF * pixel) | 0x000000FF;
            }

            SDL_UpdateTexture(p_texture, nullptr, pixels, 64 * sizeof(uint32_t));
            SDL_RenderClear(p_renderer);
            SDL_RenderCopy(p_renderer, p_texture, nullptr, nullptr);
            SDL_RenderPresent(p_renderer);
        }


        std::this_thread::sleep_for(std::chrono::microseconds(1600));
    }
    return 0;
}
