#include "now_playing_bar.h"
#include "../../app/gfx.h"
#include "../../app/hal.h"
#include "../../app/library.h"
#include "../../app/player.h"
#include "../../app/theme.h"
#include "../fonts/font_m5x7.h"
#include <stdlib.h>

static void bar_render(const widget_t *w, int16_t ax, int16_t ay) {
  const track_t *t = player_current_track(); // NULL when idle

  // background: track accent color when playing, theme surface when idle
  uint32_t bar_bg = t ? HEX_TO_RGB666((uint32_t)strtol(t->color, NULL, 16))
                      : g_theme->surface;
  gfx_draw_fill_rect(ax, ay, w->width, w->height, bar_bg);

  int16_t cover = w->height;
  if (t)
    gfx_draw_bmp(ax, ay, cover, cover, t->cover_small);
  else
    gfx_draw_fill_rect(ax, ay, cover, cover, g_theme->surface_alt);

  int16_t tx = ax + cover + 16;
  const char *title = t ? t->title : "Nothing playing";
  const char *artist = t ? t->artist : "Pick a track";
  gfx_draw_string(tx, ay + 34, title, &m5x7_font, g_theme->text, 3);
  gfx_draw_string(tx, ay + 62, artist, &m5x7_font, g_theme->text, 2);

  if (t && g_player.total_ms > 0) {
    int prog =
        (int)((uint64_t)w->width * g_player.elapsed_ms / g_player.total_ms);
    gfx_draw_fill_rect(ax, ay + w->height - 3, prog, 3, g_theme->accent);
  }
}

widget_t *now_playing_bar_create(int16_t x, int16_t y, int16_t width,
                                 int16_t height) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;
  w->x = x;
  w->y = y;
  w->width = width;
  w->height = height;
  w->visible = true;
  w->on_render = bar_render;
  return w;
}
