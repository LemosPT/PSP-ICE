#include <pspaudio.h>
#include <pspkernel.h>
#include <string.h>

#include "audio.h"

#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_FRAMES 256
#define AUDIO_CHANNELS 2
#define AUDIO_BYTES_PER_FRAME 4
#define AUDIO_PACKET_BYTES (AUDIO_FRAMES * AUDIO_BYTES_PER_FRAME)
#define AUDIO_QUEUE_SLOTS 16
#define AUDIO_PREBUFFER_PACKETS 4
#define AUDIO_VOLUME 0x4000

typedef struct {
    short samples[AUDIO_FRAMES * AUDIO_CHANNELS];
} audio_packet_t;

/* Single producer (network/main thread), single consumer (audio thread). */
static audio_packet_t queue[AUDIO_QUEUE_SLOTS];
static volatile unsigned int queue_head;
static volatile unsigned int queue_tail;
static volatile unsigned int queue_count;
static volatile int audio_running;
static int audio_channel = -1;
static SceUID audio_thread = -1;

static int audio_thread_main(SceSize args, void *argp) {
    unsigned int i;
    (void)args;
    (void)argp;

    /* Let a few packets accumulate before starting playback. */
    while (audio_running && queue_count < AUDIO_PREBUFFER_PACKETS) {
        sceKernelDelayThread(1000);
    }

    while (audio_running) {
        if (queue_count == 0) {
            /* Wait briefly for the next datagram rather than spinning. */
            sceKernelDelayThread(1000);
            continue;
        }

        sceAudioOutputPannedBlocking(
            audio_channel, AUDIO_VOLUME, AUDIO_VOLUME,
            queue[queue_tail].samples
        );

        queue_tail = (queue_tail + 1) % AUDIO_QUEUE_SLOTS;
        queue_count--;

        /* Keep the compiler from treating the queue state as unused. */
        i = queue_count;
        (void)i;
    }

    return 0;
}

int ice_audio_init(void) {
    int ret;

    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;
    audio_running = 1;

    audio_channel = sceAudioChReserve(
        PSP_AUDIO_NEXT_CHANNEL, AUDIO_FRAMES, PSP_AUDIO_FORMAT_STEREO
    );
    if (audio_channel < 0) {
        audio_running = 0;
        return audio_channel;
    }

    audio_thread = sceKernelCreateThread(
        "ice_audio", audio_thread_main, 0x18, 0x1000,
        PSP_THREAD_ATTR_USER, NULL
    );
    if (audio_thread < 0) {
        sceAudioChRelease(audio_channel);
        audio_channel = -1;
        audio_running = 0;
        return audio_thread;
    }

    ret = sceKernelStartThread(audio_thread, 0, NULL);
    if (ret < 0) {
        sceKernelDeleteThread(audio_thread);
        audio_thread = -1;
        sceAudioChRelease(audio_channel);
        audio_channel = -1;
        audio_running = 0;
        return ret;
    }

    return 0;
}

int ice_audio_queue(const void *data, unsigned int length) {
    if (!data || length != AUDIO_PACKET_BYTES || !audio_running) return -1;
    if (queue_count >= AUDIO_QUEUE_SLOTS) return -2;

    memcpy(queue[queue_head].samples, data, AUDIO_PACKET_BYTES);
    queue_head = (queue_head + 1) % AUDIO_QUEUE_SLOTS;
    queue_count++;
    return 0;
}

void ice_audio_shutdown(void) {
    audio_running = 0;

    if (audio_thread >= 0) {
        sceKernelWaitThreadEnd(audio_thread, NULL);
        sceKernelDeleteThread(audio_thread);
        audio_thread = -1;
    }

    if (audio_channel >= 0) {
        sceAudioChRelease(audio_channel);
        audio_channel = -1;
    }
}

int ice_audio_test_tone(void) {
    short buffer[512 * 2];
    float phase = 0.0f;
    const float phase_step = (2.0f * 3.14159265f * 440.0f) / AUDIO_SAMPLE_RATE;
    int channel;
    int i;
    int ret;

    channel = sceAudioChReserve(
        PSP_AUDIO_NEXT_CHANNEL, 512, PSP_AUDIO_FORMAT_STEREO
    );
    if (channel < 0) return channel;

    while (1) {
        for (i = 0; i < 512; i++) {
            short sample = (short)(sinf(phase) * 8000.0f);
            buffer[i * 2] = sample;
            buffer[i * 2 + 1] = sample;
            phase += phase_step;
            if (phase >= 2.0f * 3.14159265f) phase -= 2.0f * 3.14159265f;
        }

        ret = sceAudioOutputPannedBlocking(channel, 0x4000, 0x4000, buffer);
        if (ret < 0) {
            sceAudioChRelease(channel);
            return ret;
        }
    }

    sceAudioChRelease(channel);
    return 0;
}
