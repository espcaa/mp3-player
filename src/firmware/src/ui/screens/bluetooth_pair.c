#include "bluetooth_pair.h"
#include "../../app/gfx.h"
#include "../../app/hal.h"
#include "../../app/hal_bluetooth.h"
#include "../../app/theme.h"
#include "../components/list.h"
#include "../fonts/font_m5x7.h"
#include "../layout/container.h"
#include "../screen.h"
#include <stdio.h>
#include <stdlib.h>

#define SCAN_MS 10000
#define PANEL_X 40
#define PANEL_Y 120
#define PANEL_W 400
#define PANEL_H 400

typedef enum { PAIR_IDLE, PAIR_SCANNING, PAIR_RESULTS } pair_phase_t;

typedef struct {
  screen_manager_t *sm;
  pair_phase_t phase;
  uint32_t scan_start;
  widget_t *root;
  widget_t *list;
  char labels[MAX_BT_DEVICES][40];
  const char *items[MAX_BT_DEVICES + 2];
  int count;
} pair_ctx_t;

static void panel_render(const widget_t *w, int16_t ax, int16_t ay) {
  gfx_draw_fill_rect(ax, ay, w->width, w->height, g_theme->surface);
  gfx_draw_string(ax + 24, ay + 52, "Add a device", &m5x7_font, g_theme->text,
                  3);
}

static widget_t *panel_create(void) {
  widget_t *w = calloc(1, sizeof(widget_t));
  if (!w)
    return NULL;
  w->x = PANEL_X;
  w->y = PANEL_Y;
  w->width = PANEL_W;
  w->height = PANEL_H;
  w->visible = true;
  w->on_render = panel_render;
  return w;
}

static void pair_set_items(pair_ctx_t *c) {
  c->count = 0;
  if (c->phase == PAIR_IDLE) {
    c->items[c->count++] = "Scan for more devices";
    c->items[c->count++] = "< Back";
  } else if (c->phase == PAIR_SCANNING) {
    c->items[c->count++] = "Scanning...";
  } else {
    c->items[c->count++] = "Scan again";
    int n = hal_bt_scan_count();
    for (int i = 0; i < n && i < MAX_BT_DEVICES; i++) {
      bt_device_t d;
      if (hal_bt_get_scan_result(i, &d)) {
        snprintf(c->labels[i], sizeof(c->labels[i]), "%s", d.name);
        c->items[c->count++] = c->labels[i];
      }
    }
    if (n == 0)
      c->items[c->count++] = "No devices found";
    c->items[c->count++] = "< Back";
  }
}

static widget_t *pair_make_list(pair_ctx_t *c) {
  return list_create(PANEL_X + 16, PANEL_Y + 96, PANEL_W - 32, PANEL_H - 112,
                     c->items, c->count, &m5x7_font, NULL, c, 3);
}

static void pair_rebuild(pair_ctx_t *c) {
  if (c->list)
    widget_destroy(c->list);
  pair_set_items(c);
  c->list = pair_make_list(c);
  c->root->children[1] = c->list;
  c->list->parent = c->root;
  c->root->focus_index = 1;
}

static void pair_begin_scan(pair_ctx_t *c) {
  hal_bt_start_scan();
  c->phase = PAIR_SCANNING;
  c->scan_start = hal_get_time_ms();
  pair_rebuild(c);
}

static void pair_tick(screen_t *s, int32_t dt) {
  (void)dt;
  pair_ctx_t *c = (pair_ctx_t *)s->ctx;
  if (c->phase == PAIR_SCANNING &&
      hal_get_time_ms() - c->scan_start >= SCAN_MS) {
    hal_bt_stop_scan();
    c->phase = PAIR_RESULTS;
    pair_rebuild(c);
  }
}

static void pair_input(screen_t *s, const hal_input_t *in, int32_t dt) {
  (void)dt;
  pair_ctx_t *c = (pair_ctx_t *)s->ctx;

  if (in->dpad_right_pressed) {
    sm_pop(c->sm);
    return;
  }
  if (!in->dpad_center_pressed)
    return;

  int sel = list_selected(c->list);
  if (c->phase == PAIR_IDLE) {
    if (sel == 0)
      pair_begin_scan(c);
    else
      sm_pop(c->sm);
  } else if (c->phase == PAIR_RESULTS) {
    if (sel == 0)
      pair_begin_scan(c); // scan again
    else if (sel == c->count - 1)
      sm_pop(c->sm); // back
    else if (hal_bt_pair(sel - 1))
      sm_pop(c->sm); // paired + connected; return to the device list
  }
}

static void pair_destroy(screen_t *s) {
  if (!s)
    return;
  hal_bt_stop_scan();
  if (s->root)
    widget_destroy(s->root); // frees the panel and current list
  free(s->ctx);
  free(s);
}

screen_t *bluetooth_pair_screen_create(screen_manager_t *sm) {
  screen_t *s = calloc(1, sizeof(screen_t));
  if (!s)
    return NULL;
  pair_ctx_t *c = calloc(1, sizeof(pair_ctx_t));
  if (!c) {
    free(s);
    return NULL;
  }
  c->sm = sm;
  c->phase = PAIR_IDLE;
  s->ctx = c;
  s->on_input = pair_input;
  s->on_tick = pair_tick;
  s->on_destroy = pair_destroy;

  widget_t *root = container_create(SCREEN_WIDTH, SCREEN_HEIGHT, g_theme->bg);
  c->root = root;
  widget_add_child(root, panel_create()); // child 0

  pair_set_items(c);
  c->list = pair_make_list(c);
  widget_add_child(root, c->list); // child 1
  root->focus_index = 1;

  s->root = root;
  return s;
}
