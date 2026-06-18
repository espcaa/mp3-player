#include "image.h"
#include "../../app/gfx.h"
#include "theme.h"
#include <stdlib.h>

typedef struct {
  const char *path;
  uint32_t bg_color;
  uint32_t alpha_level;
} image_state_t;

static void image_render(const widget_t *w, int16_t abs_x, int16_t abs_y) {
  const image_state_t *s = (const image_state_t *)w->state;
  if (s && s->path)
    gfx_draw_alpha_bmp(abs_x, abs_y, w->width, w->height, s->path,
                       w->bg_color ? s->bg_color : g_theme->surface,
                       s->alpha_level ? s->alpha_level : 255);
}

static void image_destroy(widget_t *w) {
  free(w->state);
  w->state = NULL;
}

widget_t *image_create(const char *path, int16_t width, int16_t height) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;
  image_state_t *s = calloc(1, sizeof(image_state_t));
  if (!s) {
    free(w);
    return NULL;
  }

  s->path = path;
  w->state = s;
  w->visible = true;
  w->width = width;
  w->height = height;
  w->on_render = image_render;
  w->on_destroy = image_destroy;
  return w;
}

widget_t *transparent_image_create(const char *path, int16_t width,
                                   int16_t height, uint32_t bg_color,
                                   uint32_t alpha_level) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;
  image_state_t *s = calloc(1, sizeof(image_state_t));
  if (!s) {
    free(w);
    return NULL;
  }

  s->path = path;
  w->state = s;
  w->visible = true;
  w->width = width;
  w->height = height;
  w->on_render = image_render;
  w->on_destroy = image_destroy;
  return w;
}

void image_set_path(widget_t *w, const char *path) {
  if (!w || !w->state)
    return;
  ((image_state_t *)w->state)->path = path;
}
