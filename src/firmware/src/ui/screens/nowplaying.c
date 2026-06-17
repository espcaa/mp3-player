#include "../../app/library.h"
#include "../../app/player.h"
#include "../../app/theme.h"
#include "../../ui/fonts/font_ibmplexmono.h"
#include "../../ui/fonts/font_lexend.h"
#include "../../ui/fonts/font_lexend64.h"
#include "../app/hal.h"
#include "../components/image.h"
#include "../components/label.h"
#include "../layout/container.h"
#include "../screen.h"
#include <stdio.h>
#include <stdlib.h>

#define COVER 300

typedef struct {
  screen_manager_t *sm;
  widget_t *cover;
  widget_t *title_lbl;
  widget_t *artist_lbl;
  widget_t *time_lbl;
  char time_buf[16];
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
  format_time(c->time_buf, sizeof(c->time_buf), g_player.elapsed_ms);
  label_set_text(c->time_lbl, c->time_buf);
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

  // album art, centered near the top (path filled in per tick)
  c->cover = image_create(NULL, COVER, COVER);
  c->cover->x = (SCREEN_WIDTH - COVER) / 2;
  c->cover->y = 60;

  c->title_lbl = label_create("Nothing playing", &lexend64_font, g_theme->text,
                              1);
  c->title_lbl->x = 40;
  c->title_lbl->y = 400;
  c->artist_lbl =
      label_create("Pick a track", &lexend_font, g_theme->text_dim, 1);
  c->artist_lbl->x = 40;
  c->artist_lbl->y = 470;
  c->time_lbl = label_create("0:00", &ibmplexmono_font, g_theme->accent, 1);
  c->time_lbl->x = 40;
  c->time_lbl->y = 540;

  widget_add_child(root, c->cover);
  widget_add_child(root, c->title_lbl);
  widget_add_child(root, c->artist_lbl);
  widget_add_child(root, c->time_lbl);

  s->root = root;
  s->ctx = c;
  s->on_tick = np_tick;
  s->on_input = np_input;
  s->on_destroy = np_destroy;
  return s;
}
