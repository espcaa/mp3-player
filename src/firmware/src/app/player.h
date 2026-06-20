#ifndef PLAYER_H
#define PLAYER_H

#include "library.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int track_index; // index into g_library; -1 = nothing loaded
  uint32_t elapsed_ms;
  uint32_t total_ms;
  bool playing;
  uint8_t volume;
} player_t;

extern player_t g_player;

// NULL when nothing
const track_t *player_current_track(void);

void player_pump(void); // decode + feed audio; call every main loop iteration
void player_play_track(int index); // load + start (index into g_library)
void player_toggle(void);          // play/pause
void player_seek(uint32_t ms);
void player_next(void);
void player_prev(void);

void player_volume_up(void);   // step up, clamped to 100
void player_volume_down(void); // step down, clamped to 0

#endif
