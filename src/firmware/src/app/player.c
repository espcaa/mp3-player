#include "player.h"
#include "hal.h"
#include "library.h"
#include "mp3.h"
#include <stdlib.h>

player_t g_player = {.track_index = -1, .volume = 80};

static mp3_decoder_t *g_dec = NULL;
static int g_rate = 44100;

const track_t *player_current_track(void) {
  if (g_player.track_index < 0 || g_player.track_index >= g_library.count)
    return NULL;
  return &g_library.tracks[g_player.track_index];
}

static void close_decoder(void) {
  if (g_dec) {
    mp3_close(g_dec);
    g_dec = NULL;
  }
}

void player_play_track(int index) {
  if (index < 0 || index >= g_library.count)
    return;
  close_decoder();
  g_dec = mp3_open(g_library.tracks[index].path);
  g_player.track_index = index;
  g_player.elapsed_ms = 0;
  g_player.total_ms = g_library.tracks[index].duration_ms;
  g_player.playing = (g_dec != NULL);
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

#define VOLUME_STEP 5

void player_volume_up(void) {
  g_player.volume =
      g_player.volume > 100 - VOLUME_STEP ? 100 : g_player.volume + VOLUME_STEP;
}

void player_volume_down(void) {
  g_player.volume =
      g_player.volume < VOLUME_STEP ? 0 : g_player.volume - VOLUME_STEP;
}

void player_pump(void) {
  if (!g_player.playing || !g_dec)
    return;

  static int16_t pcm[MP3_MAX_SAMPLES];
  while (hal_audio_ready()) {
    int rate = 0, ch = 0;
    int n = mp3_read(g_dec, pcm, &rate, &ch);
    if (n <= 0) {
      close_decoder();
      player_next();
      if (!g_dec)
        g_player.playing = false;
      return;
    }
    if (rate > 0)
      g_rate = rate;

    int vol = g_player.volume;
    if (vol < 100)
      for (int i = 0; i < n; i++)
        pcm[i] = (int16_t)((int)pcm[i] * vol / 100);

    hal_audio_submit(pcm, (size_t)n);

    int frames = ch > 0 ? n / ch : n;
    g_player.elapsed_ms += (uint32_t)((int64_t)frames * 1000 / g_rate);
  }
}
