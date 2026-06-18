#include "vbox.h"
#include "../../app/gfx.h"
#include <stdlib.h>

static void vbox_render(const widget_t *w, int16_t abs_x, int16_t abs_y) {
  if (w->bg_color != 0) {
    gfx_draw_fill_rect(abs_x, abs_y, w->width, w->height, w->bg_color);
  }
}

widget_t *vbox_create(int16_t x, int16_t y, int16_t width, int16_t height) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;

  w->x = x;
  w->y = y;
  w->width = width;
  w->height = height;
  w->visible = true;
  w->on_layout = vbox_recalc_layout;
  w->on_render = vbox_render;
  return w;
}

void vbox_recalc_layout(widget_t *w) {
  if (!w)
    return;

  int16_t current_y = w->padding[0];
  int16_t available_width = w->width - w->padding[1] - w->padding[3];

  for (uint8_t i = 0; i < w->child_count; i++) {
    widget_t *child = w->children[i];
    if (!child)
      continue;

    child->x = w->padding[3];
    child->y = current_y;
    child->width = available_width;

    current_y += child->height + w->gap;
  }
}
