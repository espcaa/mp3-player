#ifndef LIBRARY_H
#define LIBRARY_H

#include "hal.h"

typedef struct {
  char path[HAL_MAX_PATH]; // /music/artist/album/01 - title.mp3
  char title[64];
  char artist[64];
  char album[64];
  int track;
  char color[8];                  // "FF8800"
  char cover_small[HAL_MAX_PATH]; // /music/artist/album/cover_80.bmp
  char cover_large[HAL_MAX_PATH]; // /music/artist/album/cover_300.bmp
} track_t;

typedef struct {
  track_t *tracks;
  int count;
  int capacity;
} library_t;

// one read only library for the whole app initialized at startup
extern library_t g_library;

void library_init(void); // scan /music into g_library
void library_free(void);

#endif
