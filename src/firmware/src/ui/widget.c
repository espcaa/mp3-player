#include "widget.h"
#include "../app/gfx.h"
#include <stdlib.h>

void widget_add_child(widget_t *parent, widget_t *child) {
  if (!parent || !child)
    return;
  if (parent->child_count >= MAX_CHILDREN)
    return;
  parent->children[parent->child_count++] = child;
  child->parent = parent;
}

void widget_layout(widget_t *w) {
  if (!w || !w->visible)
    return;
  // pre-order: a parent sizes/positions its children before they lay out their
  // own children, so nested boxes see the dimensions assigned to them.
  if (w->on_layout)
    w->on_layout(w);
  for (uint8_t i = 0; i < w->child_count; i++)
    widget_layout(w->children[i]);
}

void widget_render(const widget_t *w, int16_t parent_x, int16_t parent_y) {
  if (!w || !w->visible)
    return;
  int16_t abs_x = parent_x + w->x;
  int16_t abs_y = parent_y + w->y;

  if (w->on_render)
    w->on_render(w, abs_x, abs_y);

  for (uint8_t i = 0; i < w->child_count; i++)
    widget_render(w->children[i], abs_x, abs_y);
}

void widget_handle_input(widget_t *w, const hal_input_t *input, int32_t dt_ms) {
  if (!w || !w->visible)
    return;

  if (w->child_count > 0 && w->focus_index < w->child_count)
    widget_handle_input(w->children[w->focus_index], input, dt_ms);

  if (w->on_update)
    w->on_update(w, input, dt_ms);
}

void widget_destroy(widget_t *w) {
  if (!w)
    return;
  // destroy children first (post-order)
  for (uint8_t i = 0; i < w->child_count; i++)
    widget_destroy(w->children[i]);

  if (w->on_destroy)
    w->on_destroy(w);

  free(w->state);
  free(w);
}
