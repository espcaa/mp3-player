#include "bluetooth.h"
#include "../../app/hal.h"
#include "../../app/hal_bluetooth.h"
#include "../../app/theme.h"
#include "../components/label.h"
#include "../components/list.h"
#include "../components/statusbar.h"
#include "../fonts/font_lexend64.h"
#include "../fonts/font_m5x7.h"
#include "../layout/container.h"
#include "../screen.h"
#include "bluetooth_pair.h"
#include <stdio.h>
#include <stdlib.h>

#define HEADER_H 140

typedef struct {
  screen_manager_t *sm;
  widget_t *root;
  widget_t *list;
  char labels[MAX_BT_DEVICES + 1][48];
  const char *items[MAX_BT_DEVICES + 3];
  int count;
} bt_ctx_t;

static int bt_compose(bt_ctx_t *c) {
  int n = 0;
  c->items[n++] = "< Back";

  snprintf(c->labels[0], sizeof(c->labels[0]), "Bluetooth: %s",
           hal_bt_is_enabled() ? "On" : "Off");
  c->items[n++] = c->labels[0];

  if (hal_bt_is_enabled()) {
    int pc = hal_bt_paired_count();
    int conn = hal_bt_connected_index();
    for (int i = 0; i < pc && i < MAX_BT_DEVICES; i++) {
      bt_device_t d;
      if (!hal_bt_get_paired(i, &d))
        continue;
      if (i == conn)
        snprintf(c->labels[1 + i], sizeof(c->labels[1 + i]), "%s  - connected",
                 d.name);
      else
        snprintf(c->labels[1 + i], sizeof(c->labels[1 + i]), "%s", d.name);
      c->items[n++] = c->labels[1 + i];
    }
    c->items[n++] = "+ Pair another device";
  }
  return n;
}

static void bt_rebuild(bt_ctx_t *c, int count) {
  if (c->list)
    widget_destroy(c->list);
  c->count = count;
  c->list = list_create(0, HEADER_H, SCREEN_WIDTH, SCREEN_HEIGHT - HEADER_H,
                        c->items, count, &m5x7_font, NULL, c, 3);
  c->root->children[2] = c->list;
  c->list->parent = c->root;
  c->root->focus_index = 2;
}

static void bt_tick(screen_t *s, int32_t dt) {
  (void)dt;
  bt_ctx_t *c = (bt_ctx_t *)s->ctx;
  int n = bt_compose(c);
  if (n != c->count)
    bt_rebuild(c, n);
}

static void bt_input(screen_t *s, const hal_input_t *in, int32_t dt) {
  (void)dt;
  bt_ctx_t *c = (bt_ctx_t *)s->ctx;

  if (in->dpad_right_pressed) {
    sm_pop(c->sm);
    return;
  }
  if (!in->dpad_center_pressed)
    return;

  int sel = list_selected(c->list);
  if (sel == 0) {
    sm_pop(c->sm);
    return;
  }
  if (sel == 1) {
    hal_bt_set_enabled(!hal_bt_is_enabled());
    return;
  }
  if (!hal_bt_is_enabled())
    return;

  int pc = hal_bt_paired_count();
  if (sel == 2 + pc) {
    sm_push(c->sm, bluetooth_pair_screen_create(c->sm));
    return;
  }
  int dev = sel - 2;
  if (dev >= 0 && dev < pc) {
    if (dev == hal_bt_connected_index())
      hal_bt_disconnect();
    else
      hal_bt_connect(dev);
  }
}

static void bt_destroy(screen_t *s) {
  if (!s)
    return;
  if (s->root)
    widget_destroy(s->root);
  free(s->ctx);
  free(s);
}

screen_t *bluetooth_screen_create(screen_manager_t *sm) {
  screen_t *s = calloc(1, sizeof(screen_t));
  if (!s)
    return NULL;
  bt_ctx_t *c = calloc(1, sizeof(bt_ctx_t));
  if (!c) {
    free(s);
    return NULL;
  }
  c->sm = sm;
  s->ctx = c;
  s->on_input = bt_input;
  s->on_tick = bt_tick;
  s->on_destroy = bt_destroy;

  widget_t *root = container_create(SCREEN_WIDTH, SCREEN_HEIGHT, g_theme->bg);
  c->root = root;
  widget_t *status = status_bar_create();
  widget_t *header =
      label_create("Bluetooth", &lexend64_font, g_theme->text, 1);
  header->x = 24;
  header->y = HEADER_H - 28;

  widget_add_child(root, status); // 0
  widget_add_child(root, header); // 1

  c->count = bt_compose(c);
  c->list = list_create(0, HEADER_H, SCREEN_WIDTH, SCREEN_HEIGHT - HEADER_H,
                        c->items, c->count, &m5x7_font, NULL, c, 3);
  widget_add_child(root, c->list); // 2
  root->focus_index = 2;

  s->root = root;
  return s;
}
