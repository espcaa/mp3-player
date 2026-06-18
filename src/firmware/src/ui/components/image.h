#ifndef IMAGE_H
#define IMAGE_H

#include "../widget.h"

widget_t *image_create(const char *path, int16_t width, int16_t height);
void image_set_path(widget_t *w, const char *path);
widget_t *transparent_image_create(const char *path, int16_t width,
                                   int16_t height, uint32_t bg_color,
                                   uint32_t alpha_level);
#endif
