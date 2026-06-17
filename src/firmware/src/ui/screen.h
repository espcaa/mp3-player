#ifndef SCREEN_H
#define SCREEN_H
#include "widget.h"

typedef struct screen_s screen_t;
struct screen_s {
  widget_t *root;
  void (*on_enter)(screen_t *s);
  void (*on_exit)(screen_t *s);
  void (*on_tick)(screen_t *s, int32_t dt_ms);
  // Screen-level input, runs before the widget tree. Use for navigation
  // (push/pop) since the screen's ctx can hold a screen_manager_t*.
  void (*on_input)(screen_t *s, const hal_input_t *input, int32_t dt_ms);
  void (*on_destroy)(screen_t *s);
  void *ctx;
};

#define SCREEN_STACK_MAX 8
typedef struct {
  screen_t *stack[SCREEN_STACK_MAX];
  uint8_t depth;
} screen_manager_t;

void sm_init(screen_manager_t *sm);
void sm_push(screen_manager_t *sm, screen_t *s);
void sm_pop(screen_manager_t *sm);
screen_t *sm_top(screen_manager_t *sm);
void sm_update(screen_manager_t *sm, const hal_input_t *input, int32_t dt_ms);
void sm_render(screen_manager_t *sm);

#endif
