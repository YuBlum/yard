#ifndef __SOMETHING_H__
#define __SOMETHING_H__

#include "engine/math.h"
#include "engine/renderer.h"

struct something_data {
  struct v2 *position;
  struct v2 *size;
  struct v2u *texture_position;
  struct v2u *texture_size;
  struct color *color;
  float *depth;
  struct v2 *direction;
  uint32_t amount;
};

void something_init(struct something_data *self);
void something_update(struct something_data *self, float dt);
void something_render(struct something_data *self);

#endif/*__SOMETHING_H__*/
