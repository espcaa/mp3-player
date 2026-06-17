#include "player.h"
#include "library.h"

player_t g_player = {.track_index = -1, .volume = 80};

const track_t *player_current_track(void) {
  if (g_player.track_index < 0 || g_player.track_index >= g_library.count)
    return NULL;
  return &g_library.tracks[g_player.track_index];
}

void player_play_track(int index) {
  if (index < 0 || index >= g_library.count)
    return;
  g_player.track_index = index;
  g_player.elapsed_ms = 0;
  g_player.total_ms = 215000; // oop real duration TODO: fix ts
  g_player.playing = true;
}

void player_toggle(void) {
  if (g_player.track_index >= 0)
    g_player.playing = !g_player.playing;
}

void player_seek(uint32_t ms) {
  g_player.elapsed_ms = ms > g_player.total_ms ? g_player.total_ms : ms;
}

void player_next(void) {
  if (g_player.track_index >= 0 && g_player.track_index + 1 < g_library.count)
    player_play_track(g_player.track_index + 1);
}

void player_prev(void) {
  if (g_player.track_index > 0)
    player_play_track(g_player.track_index - 1);
}

void player_tick(int32_t dt_ms) {
  if (!g_player.playing || g_player.track_index < 0)
    return;
  g_player.elapsed_ms += dt_ms;
  if (g_player.elapsed_ms >= g_player.total_ms) {
    if (g_player.track_index + 1 < g_library.count) {
      player_play_track(g_player.track_index + 1); // auto-advance
    } else {
      g_player.elapsed_ms = g_player.total_ms;
      g_player.playing = false;
    }
  }
}
