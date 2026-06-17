#ifndef WIDGET_H
#define WIDGET_H

#include "../app/hal.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_CHILDREN 12

typedef struct widget_s widget_t;
typedef void (*widget_fn)(widget_t *w);

struct widget_s {
  int16_t x, y;
  int16_t width, height;

  widget_fn on_enter;
  widget_fn on_exit;
  void (*on_update)(widget_t *w, const hal_input_t *input, int32_t dt_ms);
  void (*on_render)(const widget_t *w, int16_t abs_x, int16_t abs_y);
  widget_fn on_destroy;

  widget_t *parent;
  widget_t *children[MAX_CHILDREN];
  uint8_t child_count;
  uint8_t focus_index;

  void *state;
  uint32_t bg_color;
  uint8_t padding[4];
  int16_t gap;
  bool visible;
};

void widget_add_child(widget_t *parent, widget_t *child);
void widget_render(const widget_t *w, int16_t parent_x, int16_t parent_y);
void widget_handle_input(widget_t *w, const hal_input_t *input, int32_t dt_ms);
void widget_destroy(widget_t *w);

#endif
