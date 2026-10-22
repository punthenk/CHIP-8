#include "Audio.h"
#include <cmath>

void Audio::audio_callback(void* userdata, Uint8* stream, int len) {
    Audio* audio = reinterpret_cast<Audio*>(userdata);

    audio->audio_handler(stream, len);
}

void Audio::audio_handler(Uint8* stream, int len) {
    Sint16* buffer = reinterpret_cast<Sint16*>(stream);
    int length = len / 2;

    for (int i = 0; i < length; i++) {
        float time = sample_index / SAMPLE_RATE;
        float wave = std::sin(2.0 * M_PI * FREQUENCY * time);

        buffer[i] = (wave > 0.0) ? AMPLITUDE : -AMPLITUDE;
        sample_index++;
    }
}
