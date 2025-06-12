#ifndef __MELEE_ENEMY_H__
#define __MELEE_ENEMY_H__

#include "yard/math.h"
#include "yard/renderer.h"

struct melee_enemy_data {
  struct v2 *position;
  float *size;
  struct v2u *texture_position;
  struct v2u *texture_size;
  struct v2 *velocity;
  float *speed;
  uint32_t *hp;
  uint32_t *damage;
  uint32_t amount;
};

void melee_enemy_init(struct melee_enemy_data *self);
void melee_enemy_update(struct melee_enemy_data *self, float dt);
void melee_enemy_render(struct melee_enemy_data *self);

#endif/*__MELEE_ENEMY_H__*/
