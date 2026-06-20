#include "about.h"
#include "../../app/hal.h"
#include "../../app/library.h"
#include "../../app/theme.h"
#include "../components/label.h"
#include "../components/list.h"
#include "../components/statusbar.h"
#include "../fonts/font_lexend64.h"
#include "../fonts/font_m5x7.h"
#include "../layout/container.h"
#include "../screen.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_H 140
#ifndef GIT_COMMIT
#define GIT_COMMIT "unknown"
#endif
#define ABOUT_ROWS 4

// 0 = back thing
typedef struct {
  screen_manager_t *sm;
  widget_t *list;
  char fw[32];
  char tracks[32];
  char battery[32];
  const char *items[ABOUT_ROWS];
} about_ctx_t;

static void about_input(screen_t *s, const hal_input_t *in, int32_t dt_ms) {
  (void)dt_ms;
  about_ctx_t *c = (about_ctx_t *)s->ctx;

  if (in->dpad_right_pressed) {
    sm_pop_to_root(c->sm);
    return;
  }
  if (in->dpad_center_pressed && list_selected(c->list) == 0)
    sm_pop(c->sm);
}

static void about_destroy(screen_t *s) {
  if (!s)
    return;
  if (s->root)
    widget_destroy(s->root);
  free(s->ctx);
  free(s);
}

screen_t *about_screen_create(screen_manager_t *sm) {
  screen_t *s = calloc(1, sizeof(screen_t));
  if (!s)
    return NULL;
  about_ctx_t *c = calloc(1, sizeof(about_ctx_t));
  if (!c) {
    free(s);
    return NULL;
  }
  c->sm = sm;
  s->ctx = c;
  s->on_input = about_input;
  s->on_destroy = about_destroy;

  hal_battery_t batt;
  hal_battery_read(&batt);

  snprintf(c->fw, sizeof(c->fw), "Commit %s", GIT_COMMIT);
  snprintf(c->tracks, sizeof(c->tracks), "Tracks: %d", g_library.count);
  snprintf(c->battery, sizeof(c->battery), "Battery: %d%% %s", batt.percent,
           batt.charging ? "(charging)" : "");
  c->items[0] = "< Back";
  c->items[1] = c->fw;
  c->items[2] = c->tracks;
  c->items[3] = c->battery;

  widget_t *root = container_create(SCREEN_WIDTH, SCREEN_HEIGHT, g_theme->bg);

  widget_t *status = status_bar_create();

  widget_t *header = label_create("About", &lexend64_font, g_theme->text, 1);
  header->x = 24;
  header->y = HEADER_H - 28;

  widget_t *list =
      list_create(0, HEADER_H, SCREEN_WIDTH, SCREEN_HEIGHT - HEADER_H, c->items,
                  ABOUT_ROWS, &m5x7_font, NULL, c, 3);
  c->list = list;

  widget_add_child(root, status);
  widget_add_child(root, header);
  widget_add_child(root, list);
  root->focus_index = 2;

  s->root = root;
  return s;
}
