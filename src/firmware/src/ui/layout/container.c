#include "container.h"
#include "../../app/gfx.h"
#include <stdlib.h>

static void container_render(const widget_t *w, int16_t abs_x, int16_t abs_y) {
  if (w->bg_color != 0) {
    gfx_draw_fill_rect(abs_x, abs_y, w->width, w->height, w->bg_color);
  }
  if (w->parent && w->parent->children[w->parent->focus_index] == w) {
    gfx_draw_rect(abs_x, abs_y, w->width, w->height, w->bg_color ^ 0x3F3F3F);
  }
}

widget_t *container_create(int16_t width, int16_t height, uint32_t bg_color) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;

  w->width = width;
  w->height = height;
  w->bg_color = bg_color;
  w->visible = true;
  w->on_render = container_render;
  return w;
}
