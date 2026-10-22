# pragma once

#include <SDL2/SDL.h>

class Audio {
public:
    const int SAMPLE_RATE = 44100;
    const int AMPLITUDE = 3000;
    const float FREQUENCY = 440.0;

    float sample_index = 0;
    static void audio_callback(void* userdata, Uint8* stream, int len);

private:
    void audio_handler(Uint8* stream, int len);
};