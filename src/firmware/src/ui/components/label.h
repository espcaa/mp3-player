#ifndef LABEL_H
#define LABEL_H
#include "../app/gfx.h"
#include "../widget.h"

widget_t *label_create(const char *text, const gfx_font_t *font, uint32_t color,
                       uint8_t scale);
void label_set_text(widget_t *w, const char *text);

#endif
