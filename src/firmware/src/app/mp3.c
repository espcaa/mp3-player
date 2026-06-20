#include "mp3.h"
#include "hal.h"
#include <stdlib.h>
#include <string.h>

#define MINIMP3_IMPLEMENTATION
#include "vendor/minimp3.h"

#define MP3_IN_BUF 16384

struct mp3_decoder {
  hal_file_t file;
  mp3dec_t dec;
  uint8_t in[MP3_IN_BUF];
  int in_size;
  int eof;
};

static void refill(mp3_decoder_t *d) {
  if (d->eof)
    return;
  size_t want = MP3_IN_BUF - d->in_size;
  size_t got = hal_fread(d->in + d->in_size, 1, want, d->file);
  d->in_size += (int)got;
  if (got < want)
    d->eof = 1;
}

mp3_decoder_t *mp3_open(const char *path) {
  hal_file_t f = hal_fopen(path, "rb");
  if (!f)
    return NULL;
  mp3_decoder_t *d = calloc(1, sizeof(*d));
  if (!d) {
    hal_fclose(f);
    return NULL;
  }
  d->file = f;
  mp3dec_init(&d->dec);
  refill(d);
  return d;
}

int mp3_read(mp3_decoder_t *d, int16_t *out, int *out_rate, int *out_channels) {
  for (;;) {
    mp3dec_frame_info_t info;
    int samples = mp3dec_decode_frame(&d->dec, d->in, d->in_size, out, &info);
    if (info.frame_bytes > 0) {
      memmove(d->in, d->in + info.frame_bytes, d->in_size - info.frame_bytes);
      d->in_size -= info.frame_bytes;
      refill(d);
    }
    if (samples > 0) {
      if (out_rate)
        *out_rate = info.hz;
      if (out_channels)
        *out_channels = info.channels;
      return samples * info.channels;
    }
    if (info.frame_bytes == 0) {
      int before = d->in_size;
      if (d->eof)
        return 0;
      refill(d);
      if (d->in_size == before)
        return 0; // no frame and can't make progress
    }
  }
}

void mp3_close(mp3_decoder_t *d) {
  if (!d)
    return;
  hal_fclose(d->file);
  free(d);
}
