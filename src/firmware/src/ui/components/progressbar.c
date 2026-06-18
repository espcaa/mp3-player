#include "progressbar.h"
#include "../../app/gfx.h"
#include <stdlib.h>

typedef struct {
  float progress; // 0.0 .. 1.0
  uint32_t track_color;
  uint32_t fill_color;
} progress_state_t;

static void progress_render(const widget_t *w, int16_t ax, int16_t ay) {
  const progress_state_t *s = (const progress_state_t *)w->state;
  if (!s)
    return;

  float p = s->progress;
  if (p < 0.0f)
    p = 0.0f;
  if (p > 1.0f)
    p = 1.0f;

  // track + elapsed fill
  gfx_draw_fill_rect(ax, ay, w->width, w->height, s->track_color);
  int16_t fill_w = (int16_t)(w->width * p + 0.5f);
  if (fill_w > 0)
    gfx_draw_fill_rect(ax, ay, fill_w, w->height, s->fill_color);

  // square knob centered on the bar at the fill head, clamped to the track
  int16_t knob = w->height * 3;
  int16_t kx = ax + fill_w - knob / 2;
  if (kx < ax)
    kx = ax;
  if (kx > ax + w->width - knob)
    kx = ax + w->width - knob;
  int16_t ky = ay + w->height / 2 - knob / 2;
  gfx_draw_fill_rect(kx, ky, knob, knob, s->fill_color);
}

static void progress_destroy(widget_t *w) {
  free(w->state);
  w->state = NULL;
}

widget_t *progress_bar_create(int16_t x, int16_t y, int16_t width,
                              int16_t height, uint32_t track_color,
                              uint32_t fill_color) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;
  progress_state_t *s = calloc(1, sizeof(progress_state_t));
  if (!s) {
    free(w);
    return NULL;
  }

  s->track_color = track_color;
  s->fill_color = fill_color;

  w->x = x;
  w->y = y;
  w->width = width;
  w->height = height;
  w->visible = true;
  w->state = s;
  w->on_render = progress_render;
  w->on_destroy = progress_destroy;
  return w;
}

void progress_bar_set(widget_t *w, float progress) {
  if (!w || !w->state)
    return;
  ((progress_state_t *)w->state)->progress = progress;
}
