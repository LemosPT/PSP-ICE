#include <pspaudio.h>
#include <pspkernel.h>
#include <math.h>

#include "audio.h"

#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_FRAMES 512
#define TONE_FREQUENCY 440.0f
#define TONE_AMPLITUDE 8000.0f
#define AUDIO_VOLUME 0x4000

#define PI 3.14159265f

int ice_audio_test_tone(void) {
    short buffer [AUDIO_FRAMES * 2];
    float phase = 0.0f;
    const float phase_step =
        (2.0f * PI * TONE_FREQUENCY) / AUDIO_SAMPLE_RATE;
    
    int channel;
    int i;
    int ret;

    channel = sceAudioChReserve(
        PSP_AUDIO_NEXT_CHANNEL,
        AUDIO_FRAMES,
        PSP_AUDIO_FORMAT_STEREO
    );

    if (channel < 0) {
        return channel;
    }

    while (1) {
        for (i = 0; i < AUDIO_FRAMES; i++) {
            short sample = (short)(TONE_AMPLITUDE * sinf(phase));
            buffer[i * 2] = sample;
            buffer[i * 2 + 1] = sample;
            phase += phase_step;
            if (phase >= 2.0f * PI) {
                phase -= 2.0f * PI;
            }
        }

        ret = sceAudioOutputPannedBlocking(
            channel,
            AUDIO_VOLUME,
            AUDIO_VOLUME,
            buffer
        );

        if (ret < 0) {
            return ret;
        }
    }