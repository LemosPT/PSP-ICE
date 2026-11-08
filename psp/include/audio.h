#ifndef ICE_AUDIO_H
#define ICE_AUDIO_H

typedef struct {
    unsigned int packets_received;
    unsigned int packets_played;
    unsigned int packets_dropped;
    unsigned int playback_errors;
    unsigned int queue_depth;
} ice_audio_stats_t;

int ice_audio_init(void);
int ice_audio_queue(const void *data, unsigned int length);
void ice_audio_get_stats(ice_audio_stats_t *stats);
void ice_audio_shutdown(void);
int ice_audio_test_tone(void);

#endif