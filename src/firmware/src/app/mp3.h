#ifndef MP3_H
#define MP3_H

#include <stdint.h>

#define MP3_MAX_SAMPLES (1152 * 2) // interleaved int16 per decoded frame

typedef struct mp3_decoder mp3_decoder_t;

mp3_decoder_t *mp3_open(const char *path);
// decodes one frame into out (must hold MP3_MAX_SAMPLES); returns interleaved
// sample count, 0 at end of stream. out_rate/out_channels may be NULL.
int mp3_read(mp3_decoder_t *d, int16_t *out, int *out_rate, int *out_channels);
void mp3_close(mp3_decoder_t *d);

#endif
