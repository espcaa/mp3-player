#include "statusbar.h"
#include "../../app/gfx.h"
#include "../../app/hal.h"
#include "../../app/theme.h"
#include "../fonts/font_m5x7.h"
#include "../icons/icons.h"
#include <stdio.h>
#include <stdlib.h>

static uint8_t g_battery_percent = 100;
static bool g_battery_charging = false;

static int16_t text_width(const char *s, const gfx_font_t *f, uint8_t scale) {
  int16_t width = 0;
  for (; *s; s++) {
    if (*s < f->start_char || *s > f->end_char)
      continue;
    width += f->glyphs[*s - f->start_char].advance * scale;
  }
  return width;
}

static void statusbar_render(const widget_t *w, int16_t ax, int16_t ay) {
  gfx_draw_fill_rect(ax, ay, w->width, w->height, g_theme->accent);

  char buf[8];
  snprintf(buf, sizeof(buf), "%u%%", g_battery_percent);

  const uint8_t scale = 2;
  const int16_t pad = 12;
  int16_t tw = text_width(buf, &m5x7_font, scale);
  int16_t tx = ax + w->width - pad - tw;
  int16_t ty = ay + 22;

  uint32_t fg = g_battery_charging ? g_theme->text : g_theme->on_accent;
  gfx_draw_string(tx, ty, buf, &m5x7_font, fg, scale);
  if (g_battery_charging) {
    gfx_draw_icon(tx - 24, ty - 16, &icon_charging, fg, scale);
  }
}

widget_t *status_bar_create() {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;
  w->x = 0;
  w->y = 0;
  w->width = SCREEN_WIDTH;
  w->height = 30;
  w->visible = true;
  w->on_render = statusbar_render;
  return w;
}

void status_bar_set_battery(uint8_t percent, bool charging) {
  g_battery_percent = percent;
  g_battery_charging = charging;
}
