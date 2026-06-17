#include "anim.h"
#include <math.h>

#define ANIM_DEFAULT_SPEED 14.0f
#define ANIM_SNAP_EPSILON 0.5f

void anim_init(anim_t *a, float value) {
  a->current = value;
  a->target = value;
  a->speed = ANIM_DEFAULT_SPEED;
}

void anim_set(anim_t *a, float target) { a->target = target; }

void anim_set_now(anim_t *a, float value) {
  a->current = value;
  a->target = value;
}

float anim_step(anim_t *a, int32_t dt_ms) {
  float t = a->speed * (dt_ms * 0.001f);
  if (t > 1.0f)
    t = 1.0f;
  a->current += (a->target - a->current) * t;
  if (fabsf(a->target - a->current) < ANIM_SNAP_EPSILON)
    a->current = a->target;
  return a->current;
}

bool anim_done(const anim_t *a) { return a->current == a->target; }
