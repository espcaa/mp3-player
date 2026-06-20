#include "settings.h"
#include "../../app/hal.h"
#include "../../app/theme.h"
#include "../components/label.h"
#include "../components/list.h"
#include "../components/statusbar.h"
#include "../fonts/font_lexend64.h"
#include "../fonts/font_m5x7.h"
#include "../layout/container.h"
#include "../screen.h"
#include "about.h"
#include "bluetooth.h"
#include <stdlib.h>

#define HEADER_H 140

enum { ROW_BLUETOOTH = 0, ROW_ABOUT };

static const char *SETTINGS_ITEMS[] = {"Bluetooth", "About"};
#define SETTINGS_COUNT (int)(sizeof(SETTINGS_ITEMS) / sizeof(SETTINGS_ITEMS[0]))

typedef struct {
  screen_manager_t *sm;
  widget_t *list;
} settings_ctx_t;

static void settings_input(screen_t *s, const hal_input_t *in, int32_t dt_ms) {
  (void)dt_ms;
  settings_ctx_t *c = (settings_ctx_t *)s->ctx;

  // dpad_right is the global back gesture, unwind to the root library screen
  if (in->dpad_right_pressed) {
    sm_pop_to_root(c->sm);
    return;
  }

  if (in->dpad_center_pressed) {
    switch (list_selected(c->list)) {
    case ROW_BLUETOOTH:
      sm_push(c->sm, bluetooth_screen_create(c->sm));
      break;
    case ROW_ABOUT:
      sm_push(c->sm, about_screen_create(c->sm));
      break;
    }
  }
}

static void settings_destroy(screen_t *s) {
  if (!s)
    return;
  if (s->root)
    widget_destroy(s->root);
  free(s->ctx);
  free(s);
}

screen_t *settings_screen_create(screen_manager_t *sm) {
  screen_t *s = calloc(1, sizeof(screen_t));
  if (!s)
    return NULL;
  settings_ctx_t *c = calloc(1, sizeof(settings_ctx_t));
  if (!c) {
    free(s);
    return NULL;
  }
  c->sm = sm;
  s->ctx = c;
  s->on_input = settings_input;
  s->on_destroy = settings_destroy;

  widget_t *root = container_create(SCREEN_WIDTH, SCREEN_HEIGHT, g_theme->bg);

  widget_t *status = status_bar_create();

  widget_t *header = label_create("Settings", &lexend64_font, g_theme->text, 1);
  header->x = 24;
  header->y = HEADER_H - 28;

  widget_t *list =
      list_create(0, HEADER_H, SCREEN_WIDTH, SCREEN_HEIGHT - HEADER_H,
                  SETTINGS_ITEMS, SETTINGS_COUNT, &m5x7_font, NULL, c, 3);
  c->list = list;

  widget_add_child(root, status);
  widget_add_child(root, header);
  widget_add_child(root, list);
  root->focus_index = 2; // route up/down to the list

  s->root = root;
  return s;
}
