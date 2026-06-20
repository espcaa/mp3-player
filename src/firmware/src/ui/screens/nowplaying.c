#include "../../app/library.h"
#include "../../app/player.h"
#include "../../app/theme.h"
#include "../../ui/fonts/font_lexend.h"
#include "../../ui/fonts/font_m5x7.h"
#include "../app/hal.h"
#include "../components/image.h"
#include "../components/label.h"
#include "../components/progressbar.h"
#include "../components/statusbar.h"
#include "../layout/container.h"
#include "../screen.h"
#include <stdio.h>
#include <stdlib.h>

#define COVER 300
#define BOTTOM_Y 400 // top of the panel below the album art

typedef struct {
  screen_manager_t *sm;
  widget_t *cover;
  widget_t *title_lbl;
  widget_t *artist_lbl;
  widget_t *progress;
  widget_t *elapsed_lbl;
  widget_t *total_lbl;
  char elapsed_buf[16];
  char total_buf[16];
} now_playing_ctx_t;

static void format_time(char *buf, size_t n, uint32_t ms) {
  uint32_t total_s = ms / 1000;
  snprintf(buf, n, "%lu:%02lu", (unsigned long)(total_s / 60),
           (unsigned long)(total_s % 60));
}

static void np_tick(screen_t *s, int32_t dt_ms) {
  (void)dt_ms;
  now_playing_ctx_t *c = (now_playing_ctx_t *)s->ctx;
  const track_t *t = player_current_track();
  label_set_text(c->title_lbl, t ? t->title : "Nothing playing");
  label_set_text(c->artist_lbl, t ? t->artist : "Pick a track");
  image_set_path(c->cover, t ? t->cover_large : NULL);
  s->root->bg_color =
      t ? HEX_TO_RGB666((uint32_t)strtol(t->color, NULL, 16)) : g_theme->bg;

  float p = g_player.total_ms > 0
                ? (float)g_player.elapsed_ms / (float)g_player.total_ms
                : 0.0f;
  progress_bar_set(c->progress, p);

  format_time(c->elapsed_buf, sizeof(c->elapsed_buf), g_player.elapsed_ms);
  format_time(c->total_buf, sizeof(c->total_buf), g_player.total_ms);
  label_set_text(c->elapsed_lbl, c->elapsed_buf);
  label_set_text(c->total_lbl, c->total_buf);
}

static void np_input(screen_t *s, const hal_input_t *in, int32_t dt_ms) {
  (void)dt_ms;
  now_playing_ctx_t *c = (now_playing_ctx_t *)s->ctx;
  if (in->dpad_left_pressed)
    sm_pop(c->sm); // back to the library
  else if (in->dpad_center_pressed)
    player_toggle();
  else if (in->dpad_up_pressed)
    player_prev();
  else if (in->dpad_down_pressed)
    player_next();
}

static void np_destroy(screen_t *s) {
  if (!s)
    return;
  if (s->root)
    widget_destroy(s->root);
  free(s->ctx);
  free(s);
}

screen_t *now_playing_screen_create(screen_manager_t *sm) {
  screen_t *s = calloc(1, sizeof(screen_t));
  if (!s)
    return NULL;
  now_playing_ctx_t *c = calloc(1, sizeof(now_playing_ctx_t));
  if (!c) {
    free(s);
    return NULL;
  }
  c->sm = sm;

  widget_t *root = container_create(SCREEN_WIDTH, SCREEN_HEIGHT, g_theme->bg);

  widget_t *status = status_bar_create();

  // album art, centered near the top (path filled in per tick)
  c->cover = transparent_image_create(NULL, COVER, COVER, g_theme->bg, 100);
  c->cover->x = (SCREEN_WIDTH - COVER) / 2;
  c->cover->y = 60;

  // panel holding everything below the album art; coords below are relative
  widget_t *bottom = container_create(SCREEN_WIDTH, SCREEN_HEIGHT - BOTTOM_Y,
                                      g_theme->surface);
  bottom->y = BOTTOM_Y;

  const int16_t pad = 36; // side margin inside the panel
  const int16_t content_w = SCREEN_WIDTH - pad * 2;

  c->title_lbl = label_create("Nothing playing", &m5x7_font, g_theme->text, 3);
  c->title_lbl->width = SCREEN_WIDTH - pad * 2;
  c->title_lbl->x = pad;
  c->title_lbl->y = (52);
  c->artist_lbl =
      label_create("Pick a track", &m5x7_font, g_theme->text_dim, 3);
  c->artist_lbl->x = pad;
  c->artist_lbl->width = SCREEN_WIDTH - pad * 2;
  c->artist_lbl->y = 92;

  const int16_t bar_y = 130;
  c->progress = progress_bar_create(pad, bar_y, content_w, 6,
                                    g_theme->surface_alt, g_theme->accent);

  c->elapsed_lbl = label_create("0:00", &m5x7_font, g_theme->text_dim, 2);
  c->elapsed_lbl->x = pad;
  c->elapsed_lbl->y = bar_y + 40;
  c->total_lbl = label_create("0:00", &m5x7_font, g_theme->text_dim, 2);
  c->total_lbl->x = SCREEN_WIDTH - pad - 64;
  c->total_lbl->y = bar_y + 40;

  widget_add_child(root, status);
  widget_add_child(root, c->cover);
  widget_add_child(root, bottom);
  widget_add_child(bottom, c->title_lbl);
  widget_add_child(bottom, c->artist_lbl);
  widget_add_child(bottom, c->progress);
  widget_add_child(bottom, c->elapsed_lbl);
  widget_add_child(bottom, c->total_lbl);

  s->root = root;
  s->ctx = c;
  s->on_tick = np_tick;
  s->on_input = np_input;
  s->on_destroy = np_destroy;
  return s;
}
