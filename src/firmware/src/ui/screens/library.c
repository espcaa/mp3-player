#include "library.h"
#include "../../app/hal.h"
#include "../../app/player.h"
#include "../../app/theme.h"
#include "../components/label.h"
#include "../components/list.h"
#include "../components/now_playing_bar.h"
#include "../components/statusbar.h"
#include "../fonts/font_lexend64.h"
#include "../fonts/font_m5x7.h"
#include "../layout/container.h"
#include "nowplaying.h"
#include <stdlib.h>

#define HEADER_H 140
#define BAR_H 88

typedef struct {
  screen_manager_t *sm;
  const char **titles; // item array for the list; points into g_library
} library_ctx_t;

static const char *EMPTY_ITEMS[] = {"No music found"};

static void on_track_select(int index, void *user) {
  library_ctx_t *c = (library_ctx_t *)user;
  if (index < 0 || index >= g_library.count)
    return;
  player_play_track(index);
}

static void lib_input(screen_t *s, const hal_input_t *in, int32_t dt_ms) {
  (void)dt_ms;
  library_ctx_t *c = (library_ctx_t *)s->ctx;
  // open the full now-playing screen (shows a placeholder when idle)
  if (in->dpad_right_pressed)
    sm_push(c->sm, now_playing_screen_create(c->sm));
}

static void lib_destroy(screen_t *s) {
  if (!s)
    return;
  if (s->root)
    widget_destroy(s->root); // destroys the list before its items are freed
  library_ctx_t *c = (library_ctx_t *)s->ctx;
  if (c)
    free(c->titles);
  free(c);
  free(s);
}

screen_t *library_screen_create(screen_manager_t *sm) {
  screen_t *s = calloc(1, sizeof(screen_t));
  if (!s)
    return NULL;
  library_ctx_t *c = calloc(1, sizeof(library_ctx_t));
  if (!c) {
    free(s);
    return NULL;
  }
  c->sm = sm;

  // List items point straight at the persistent global library titles.
  const char **items = EMPTY_ITEMS;
  int count = 1;
  if (g_library.count > 0) {
    c->titles = malloc(g_library.count * sizeof(char *));
    if (c->titles) {
      for (int i = 0; i < g_library.count; i++)
        c->titles[i] = g_library.tracks[i].title;
      items = c->titles;
      count = g_library.count;
    }
  }

  widget_t *root = container_create(SCREEN_WIDTH, SCREEN_HEIGHT, g_theme->bg);

  widget_t *status = status_bar_create();

  widget_t *header = label_create("Library", &lexend64_font, g_theme->text, 1);
  header->x = 24;
  header->y = HEADER_H - 28;

  widget_t *list =
      list_create(0, HEADER_H, SCREEN_WIDTH, SCREEN_HEIGHT - HEADER_H - BAR_H,
                  items, count, &m5x7_font, on_track_select, c, 3);

  widget_t *bar =
      now_playing_bar_create(0, SCREEN_HEIGHT - BAR_H, SCREEN_WIDTH, BAR_H);

  widget_add_child(root, status);
  widget_add_child(root, header);
  widget_add_child(root, list);
  widget_add_child(root, bar);
  // route input to the list (child index 2: status, header, list, bar)
  root->focus_index = 2;

  s->root = root;
  s->ctx = c;
  s->on_input = lib_input;
  s->on_destroy = lib_destroy;
  return s;
}
