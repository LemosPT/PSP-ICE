#ifndef ICE_AUDIO_H
#define ICE_AUDIO_H

int ice_audio_init(void);
int ice_audio_queue(const void *data, unsigned int length);
void ice_audio_shutdown(void);
int ice_audio_test_tone(void);

#endif
