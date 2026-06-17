#ifndef ANIM_H
#define ANIM_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  float current;
  float target;
  float speed;
} anim_t;

void anim_init(anim_t *a, float value);
void anim_set(anim_t *a, float target);
void anim_set_now(anim_t *a, float value);
float anim_step(anim_t *a, int32_t dt_ms);
bool anim_done(const anim_t *a);

#endif
