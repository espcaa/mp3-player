#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "../widget.h"

// A horizontal progress bar with a draggable-looking knob at the fill head.
// `progress` is clamped to 0.0..1.0.
widget_t *progress_bar_create(int16_t x, int16_t y, int16_t width,
                              int16_t height, uint32_t track_color,
                              uint32_t fill_color);

void progress_bar_set(widget_t *w, float progress);

#endif
