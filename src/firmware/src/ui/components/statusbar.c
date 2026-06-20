#include "statusbar.h"
#include "../../app/gfx.h"
#include "../../app/hal.h"
#include "../../app/hal_bluetooth.h"
#include "../../app/player.h"
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

static const gfx_icon_t *volume_icon(uint8_t volume) {
  if (volume == 0)
    return &icon_sound_muted;
  if (volume < 34)
    return &icon_sound_low;
  if (volume < 67)
    return &icon_sound_medium;
  return &icon_sound_high;
}

static int16_t draw_icon_rtl(int16_t x_right, int16_t ay, int16_t bar_h,
                             const gfx_icon_t *icon, uint32_t color,
                             uint8_t scale) {
  int16_t ix = x_right - icon->width * scale;
  int16_t iy = ay + (bar_h - icon->height * scale) / 2;
  gfx_draw_icon(ix, iy, icon, color, scale);
  return ix;
}

static int16_t draw_pct_rtl(int16_t x_right, int16_t ty, const char *text,
                            uint32_t color, uint8_t scale, int16_t field_w) {
  int16_t tw = text_width(text, &m5x7_font, scale);
  gfx_draw_string(x_right - tw, ty, text, &m5x7_font, color, scale);
  return x_right - field_w;
}

// draw 2px wide divider
static int16_t draw_divider(int16_t x_right, int16_t ay, int16_t bar_h) {
  const int16_t dw = 2;
  gfx_draw_fill_rect(x_right - dw, ay, dw, bar_h, g_theme->on_accent);
  return x_right - dw;
}

static void statusbar_render(const widget_t *w, int16_t ax, int16_t ay) {
  gfx_draw_fill_rect(ax, ay, w->width, w->height, g_theme->accent);

  const uint8_t scale = 2;
  const int16_t pad = 12;
  const int16_t gap = 8;
  const int16_t div_gap = 14;
  const int16_t ty = ay + 22;
  const uint32_t fg = g_theme->on_accent;

  const int16_t pct_w = text_width("100%", &m5x7_font, scale);

  int16_t batt_icon_w = icon_battery.width > icon_charging.width
                            ? icon_battery.width
                            : icon_charging.width;
  batt_icon_w *= scale;

  char buf[8];
  int16_t cursor = ax + w->width - pad;

  const uint32_t batt_fg = g_battery_charging ? g_theme->text : fg;
  const gfx_icon_t *batt_icon =
      g_battery_charging ? &icon_charging : &icon_battery;
  draw_icon_rtl(cursor, ay, w->height, batt_icon, batt_fg, scale);
  cursor -= batt_icon_w + gap;
  snprintf(buf, sizeof(buf), "%u%%", g_battery_percent);
  cursor = draw_pct_rtl(cursor, ty, buf, batt_fg, scale, pct_w);

  cursor -= div_gap;
  cursor = draw_divider(cursor, ay, w->height);
  cursor -= div_gap;

  cursor = draw_icon_rtl(cursor, ay, w->height, volume_icon(g_player.volume),
                         fg, scale);
  cursor -= gap;
  snprintf(buf, sizeof(buf), "%u%%", g_player.volume);
  cursor = draw_pct_rtl(cursor, ty, buf, fg, scale, pct_w);

  cursor -= div_gap;
  int16_t left_div = draw_divider(cursor, ay, w->height);

  if (hal_bt_is_enabled()) {
    int16_t lx = ax + pad;
    int16_t iy = ay + (w->height - icon_bluetooth.height * scale) / 2;
    gfx_draw_icon(lx, iy, &icon_bluetooth, fg, scale);
    lx += icon_bluetooth.width * scale + gap;

    int conn = hal_bt_connected_index();
    bt_device_t d;
    if (conn >= 0 && hal_bt_get_paired(conn, &d)) {
      int16_t name_max = left_div - div_gap - lx;
      if (name_max > 0)
        gfx_draw_string_ellipsized(lx, ty, d.name, &m5x7_font, fg, scale,
                                   name_max);
    }
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
