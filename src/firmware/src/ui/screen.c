#include "screen.h"
#include "../app/gfx.h"

void sm_init(screen_manager_t *sm) { sm->depth = 0; }

screen_t *sm_top(screen_manager_t *sm) {
  return sm->depth ? sm->stack[sm->depth - 1] : NULL;
}

void sm_push(screen_manager_t *sm, screen_t *s) {
  if (!s || sm->depth >= SCREEN_STACK_MAX)
    return;
  screen_t *cur = sm_top(sm);
  if (cur && cur->on_exit)
    cur->on_exit(cur);
  sm->stack[sm->depth++] = s;
  if (s->on_enter)
    s->on_enter(s);
}

void sm_pop(screen_manager_t *sm) {
  if (sm->depth == 0)
    return;
  screen_t *top = sm->stack[--sm->depth];
  if (top->on_exit)
    top->on_exit(top);
  if (top->on_destroy)
    top->on_destroy(top);
  screen_t *below = sm_top(sm);
  if (below && below->on_enter)
    below->on_enter(below);
}

void sm_update(screen_manager_t *sm, const hal_input_t *input, int32_t dt_ms) {
  screen_t *top = sm_top(sm);
  if (!top)
    return;
  if (top->on_tick)
    top->on_tick(top, dt_ms);
  if (top->on_input) {
    top->on_input(top, input, dt_ms);
    // on_input may have pushed/popped (and freed `top`); bail if so to avoid
    // feeding input to a stale screen this frame.
    if (sm_top(sm) != top)
      return;
  }
  if (top->root)
    widget_handle_input(top->root, input, dt_ms);
}

void sm_render(screen_manager_t *sm) {
  screen_t *top = sm_top(sm);
  if (top && top->root) {
    widget_layout(top->root);
    widget_render(top->root, 0, 0);
  }
}
