#ifndef HBOX_H
#define HBOX_H

#include "../widget.h"

widget_t *hbox_create(int16_t x, int16_t y, int16_t width, int16_t height);

void hbox_recalc_layout(widget_t *w);

#endif
