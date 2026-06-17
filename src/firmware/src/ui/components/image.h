#ifndef IMAGE_H
#define IMAGE_H

#include "../widget.h"

widget_t *image_create(const char *path, int16_t width, int16_t height);
void image_set_path(widget_t *w, const char *path);

#endif
