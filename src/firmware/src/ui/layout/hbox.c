#include "hbox.h"
#include "../../app/gfx.h"
#include <stdlib.h>

static void hbox_render(const widget_t *w, int16_t abs_x, int16_t abs_y) {
  if (w->bg_color != 0) {
    gfx_draw_fill_rect(abs_x, abs_y, w->width, w->height, w->bg_color);
  }
}

widget_t *hbox_create(int16_t x, int16_t y, int16_t width, int16_t height) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;

  w->x = x;
  w->y = y;
  w->width = width;
  w->height = height;
  w->visible = true;
  w->on_layout = hbox_recalc_layout;
  w->on_render = hbox_render;
  return w;
}

void hbox_recalc_layout(widget_t *w) {
  if (!w)
    return;

  int16_t current_x = w->padding[3];
  int16_t available_height = w->height - w->padding[0] - w->padding[2];

  for (uint8_t i = 0; i < w->child_count; i++) {
    widget_t *child = w->children[i];
    if (!child)
      continue;

    // children keep their own width; stretch to fill height
    child->x = current_x;
    child->y = w->padding[0];
    child->height = available_height;

    current_x += child->width + w->gap;
  }
}
