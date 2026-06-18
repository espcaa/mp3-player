#ifndef LIST_H
#define LIST_H

#include "../../app/gfx.h"
#include "../widget.h"

typedef void (*list_select_fn)(int index, void *user);

widget_t *list_create(int16_t x, int16_t y, int16_t width, int16_t height,
                      const char **items, int count, const gfx_font_t *font,
                      list_select_fn on_select, void *user, int font_scale);

int list_selected(const widget_t *w);

#endif
