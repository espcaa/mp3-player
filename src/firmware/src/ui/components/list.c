#include "list.h"
#include "../../app/gfx.h"
#include "../../app/hal.h"
#include "../../app/theme.h"
#include "../anim.h"
#include <stdlib.h>

#define ITEM_PAD_Y 32
#define ITEM_PAD_X 24

typedef struct {
  const char **items;
  int count;
  int selected;
  const gfx_font_t *font;
  int16_t item_height;
  anim_t hl_y;
  anim_t scroll;
  list_select_fn on_select;
  void *user;
  int font_scale;
} list_state_t;

static void update_scroll_target(list_state_t *s, int16_t view_h) {
  float top = (float)(s->selected * s->item_height);
  float bot = top + s->item_height;
  float scr = s->scroll.target;
  if (top < scr)
    scr = top;
  if (bot > scr + view_h)
    scr = bot - view_h;

  float max = (float)(s->count * s->item_height - view_h);
  if (max < 0.0f)
    max = 0.0f;
  if (scr > max)
    scr = max;
  if (scr < 0.0f)
    scr = 0.0f;
  anim_set(&s->scroll, scr);
}

static void list_update(widget_t *w, const hal_input_t *in, int32_t dt) {
  list_state_t *s = (list_state_t *)w->state;

  if (in->dpad_down_pressed && s->selected < s->count - 1)
    s->selected++;
  if (in->dpad_up_pressed && s->selected > 0)
    s->selected--;
  if (in->dpad_center_pressed && s->on_select)
    s->on_select(s->selected, s->user);

  anim_set(&s->hl_y, (float)(s->selected * s->item_height));
  update_scroll_target(s, w->height);
  anim_step(&s->hl_y, dt);
  anim_step(&s->scroll, dt);
}

static void list_render(const widget_t *w, int16_t ax, int16_t ay) {
  const list_state_t *s = (const list_state_t *)w->state;

  gfx_set_clip(ax, ay, w->width, w->height);
  gfx_draw_fill_rect(ax, ay, w->width, w->height, g_theme->bg);

  int16_t scroll = (int16_t)(s->scroll.current + 0.5f);
  int16_t hl_y = (int16_t)(s->hl_y.current + 0.5f);

  gfx_draw_fill_rect(ax, ay + hl_y - scroll, w->width, s->item_height,
                     g_theme->accent);

  for (int i = 0; i < s->count; i++) {
    int16_t row_y = ay + (int16_t)(i * s->item_height) - scroll;
    if (row_y + s->item_height <= ay || row_y >= ay + w->height)
      continue;
    int16_t baseline = row_y + ITEM_PAD_Y + s->font->height;
    uint32_t col = (i == s->selected) ? g_theme->on_accent : g_theme->text_dim;
    int16_t max_width = w->width - ITEM_PAD_X * 2;
    gfx_draw_string_ellipsized(ax + ITEM_PAD_X, baseline + ITEM_PAD_Y / 2.5,
                               s->items[i], s->font, col, s->font_scale,
                               max_width);
  }

  gfx_clear_clip();
}

static void list_destroy(widget_t *w) {
  free(w->state);
  w->state = NULL;
}

widget_t *list_create(int16_t x, int16_t y, int16_t width, int16_t height,
                      const char **items, int count, const gfx_font_t *font,
                      list_select_fn on_select, void *user, int font_scale) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;
  list_state_t *s = calloc(1, sizeof(list_state_t));
  if (!s) {
    free(w);
    return NULL;
  }

  s->items = items;
  s->font_scale = font_scale;
  s->count = count;
  s->font = font;
  s->item_height = (int16_t)(font->height + ITEM_PAD_Y * 2);
  s->on_select = on_select;
  s->user = user;

  anim_init(&s->hl_y, 0.0f);
  anim_init(&s->scroll, 0.0f);

  w->x = x;
  w->y = y;
  w->width = width;
  w->height = height;
  w->visible = true;
  w->state = s;
  w->on_render = list_render;
  w->on_update = list_update;
  w->on_destroy = list_destroy;
  return w;
}

int list_selected(const widget_t *w) {
  return (w && w->state) ? ((const list_state_t *)w->state)->selected : 0;
}
