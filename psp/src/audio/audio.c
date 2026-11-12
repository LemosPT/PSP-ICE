
#include <pspaudio.h>
#include <pspkernel.h>
#include <string.h>
#include <math.h>

#include "audio.h"

#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_FRAMES 256
#define AUDIO_CHANNELS 2
#define AUDIO_BYTES_PER_FRAME 4
#define AUDIO_PACKET_BYTES (AUDIO_FRAMES * AUDIO_BYTES_PER_FRAME)
#define AUDIO_QUEUE_SLOTS 16
#define AUDIO_PREBUFFER_PACKETS 4
#define AUDIO_VOLUME 0x8000


typedef struct {
    short samples[AUDIO_FRAMES * AUDIO_CHANNELS];
} audio_packet_t;

/* Network/main thread produces; audio thread consumes. */
static audio_packet_t queue[AUDIO_QUEUE_SLOTS];
static unsigned int queue_head;
static unsigned int queue_tail;
static unsigned int queue_count;
static unsigned int packets_received;
static unsigned int packets_played;
static unsigned int packets_dropped;
static unsigned int playback_errors;

static int audio_running;

static int audio_channel = -1;
static int audio_mutex = -1;
static SceUID audio_thread = -1;

/* Queue state and packet copies are protected by this mutex. */
static int audio_lock(void) {
    if (audio_mutex < 0) return -1;

    return sceKernelWaitSema(audio_mutex, 1, NULL);
}

static void audio_unlock(void) {
    if (audio_mutex >= 0) {
        sceKernelSignalSema(audio_mutex, 1);
    }
}

static int audio_thread_main(SceSize args, void *argp) {
    short samples[AUDIO_FRAMES * AUDIO_CHANNELS];
    int running;
    int ready;
    int has_packet;
    int ret;
    (void)args;
    (void)argp;

    /* Wait until the prebuffer is full before starting playback. */
    while (1) {
        if (audio_lock() < 0) break;

        running = audio_running;
        ready = queue_count >= AUDIO_PREBUFFER_PACKETS;

        audio_unlock();

        if (!running || ready) break;
        sceKernelDelayThread(1000);
    }

    while (1) {
        if (audio_lock() < 0) break;

        running = audio_running;
        has_packet = 0;

        if (running && queue_count > 0) {
            memcpy(samples, queue[queue_tail].samples,
                   AUDIO_PACKET_BYTES);

            queue_tail = (queue_tail + 1) % AUDIO_QUEUE_SLOTS;
            queue_count--;
            has_packet = 1;
        }

        audio_unlock();

        if (!running) break;

        if (!has_packet) {
            sceKernelDelayThread(1000);
            continue;
        }

        /* Never hold the queue mutex during blocking audio playback. */
        int ret = sceAudioOutputPannedBlocking(
            audio_channel, AUDIO_VOLUME, AUDIO_VOLUME, samples
        );
        
        if (audio_lock() >= 0) {
            if (ret < 0) {
                playback_errors++;
            } else {
                packets_played++;
            }
            
            audio_unlock();
        }
    }

    return 0;
}

int ice_audio_init(void) {
    int ret;

    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;
    audio_running = 1;
    packets_received = 0;
    packets_played = 0;
    packets_dropped = 0;
    playback_errors = 0;

    audio_mutex = sceKernelCreateSema(
        "ice_audio_mutex", 0, 1, 1, NULL
    );
    if (audio_mutex < 0) {
        audio_running = 0;
        return audio_mutex;
    }

    audio_channel = sceAudioChReserve(
        PSP_AUDIO_NEXT_CHANNEL, AUDIO_FRAMES, PSP_AUDIO_FORMAT_STEREO
    );
    if (audio_channel < 0) {
        sceKernelDeleteSema(audio_mutex);
        audio_mutex = -1;
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
        sceKernelDeleteSema(audio_mutex);
        audio_mutex = -1;
        audio_running = 0;
        return audio_thread;
    }

    ret = sceKernelStartThread(audio_thread, 0, NULL);
    if (ret < 0) {
        sceKernelDeleteThread(audio_thread);
        audio_thread = -1;
        sceAudioChRelease(audio_channel);
        audio_channel = -1;
        sceKernelDeleteSema(audio_mutex);
        audio_mutex = -1;
        audio_running = 0;
        return ret;
    }

    return 0;
}

int ice_audio_queue(const void *data, unsigned int length) {
    if (!data || length != AUDIO_PACKET_BYTES) return -1;

    if (audio_lock() < 0) return -3;

    if (!audio_running) {
        audio_unlock();
        return -1;
    }

    packets_received++;

    if (queue_count >= AUDIO_QUEUE_SLOTS) {
        queue_tail = (queue_tail + 1) % AUDIO_QUEUE_SLOTS;
        queue_count--;
        packets_dropped++;
    }

    memcpy(queue[queue_head].samples, data, AUDIO_PACKET_BYTES);

    queue_head = (queue_head + 1) % AUDIO_QUEUE_SLOTS;
    queue_count++;

    audio_unlock();
    return 0;
}

void ice_audio_get_stats(ice_audio_stats_t *stats) {
    if (!stats) return;

    stats->packets_received = 0;
    stats->packets_played = 0;
    stats->packets_dropped = 0;
    stats->playback_errors = 0;
    stats->queue_depth = 0;

    if (audio_lock() < 0) return;

    stats->packets_received = packets_received;
    stats->packets_played = packets_played;
    stats->packets_dropped = packets_dropped;
    stats->playback_errors = playback_errors;
    stats->queue_depth = queue_count;

    audio_unlock();
}

void ice_audio_shutdown(void) {
    if (audio_mutex >= 0) {
        if (audio_lock() >= 0) {
            audio_running = 0;
            audio_unlock();
        }
    }

    if (audio_thread >= 0) {
        sceKernelWaitThreadEnd(audio_thread, NULL);
        sceKernelDeleteThread(audio_thread);
        audio_thread = -1;
    }

    if (audio_channel >= 0) {
        sceAudioChRelease(audio_channel);
        audio_channel = -1;
    }

    if (audio_mutex >= 0) {
        sceKernelDeleteSema(audio_mutex);
        audio_mutex = -1;
    }
}

int ice_audio_test_tone(void) {
    short buffer[512 * 2];
    float phase = 0.0f;
    const float phase_step =
        (2.0f * 3.14159265f * 440.0f) / AUDIO_SAMPLE_RATE;
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
            if (phase >= 2.0f * 3.14159265f) {
                phase -= 2.0f * 3.14159265f;
            }
        }

        ret = sceAudioOutputPannedBlocking(
            channel, 0x4000, 0x4000, buffer
        );

        if (ret < 0) {
            sceAudioChRelease(channel);
            return ret;
        }
    }

    sceAudioChRelease(channel);
    return 0;
}
