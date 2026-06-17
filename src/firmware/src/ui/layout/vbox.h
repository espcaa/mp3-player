
#ifndef VBOX_H
#define VBOX_H

#include "../widget.h"

widget_t *vbox_create(int16_t x, int16_t y, int16_t width, int16_t height);

void vbox_recalc_layout(widget_t *w);

#endif
