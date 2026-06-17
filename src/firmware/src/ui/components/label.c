#include "label.h"
#include <stdlib.h>

typedef struct {
  const char *text;
  const gfx_font_t *font;
  uint32_t color;
  uint8_t scale;
} label_state_t;

static void label_render(const widget_t *w, int16_t abs_x, int16_t abs_y) {
  const label_state_t *s = (const label_state_t *)w->state;
  if (!s || !s->text)
    return;
  int16_t baseline = abs_y + (s->font->height * s->scale);
  gfx_draw_string(abs_x, baseline, s->text, s->font, s->color, s->scale);
}

static void label_destroy(widget_t *w) {
  free(w->state);
  w->state = NULL;
}

widget_t *label_create(const char *text, const gfx_font_t *font, uint32_t color,
                       uint8_t scale) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;
  label_state_t *s = calloc(1, sizeof(label_state_t));
  if (!s) {
    free(w);
    return NULL;
  }

  s->text = text;
  s->font = font;
  s->color = color;
  s->scale = scale;

  w->state = s;
  w->visible = true;
  w->on_render = label_render;
  w->on_destroy = label_destroy;
  w->height = font->height * scale;
  return w;
}

void label_set_text(widget_t *w, const char *text) {
  if (!w || !w->state)
    return;
  ((label_state_t *)w->state)->text = text;
}
