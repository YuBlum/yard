#include <stdlib.h>
#include "yard/entities/something.h"

void
something_init(struct something_data *self) {
  for (uint32_t i = 0; i < self->amount; i++) {
    self->size[i].x = 0.25f + 0.75f * ((float)rand() / (float)RAND_MAX);
    self->size[i].y = 0.25f + 0.75f * ((float)rand() / (float)RAND_MAX);
  }
  for (uint32_t i = 0; i < self->amount; i++) {
    self->color[i].r = ((float)rand() / (float)RAND_MAX);
    self->color[i].g = ((float)rand() / (float)RAND_MAX);
    self->color[i].b = ((float)rand() / (float)RAND_MAX);
  }
  for (uint32_t i = 0; i < self->amount; i++) {
    self->texture_size[i] = V2U(1, 1);
  }
  for (uint32_t i = 0; i < self->amount; i++) {
    self->texture_position[i] = ATLAS_PIXEL_POS;
  }
  for (uint32_t i = 0; i < self->amount; i++) {
    float angle = ((float)rand() / (float)RAND_MAX) * 2.0f * PI;
    float speed = 1.0f + ((float)rand() / (float)RAND_MAX) * 4.0f;
    self->direction[i].x = cosf(angle) * speed;
    self->direction[i].y = sinf(angle) * speed;
  }
}

void
something_update(struct something_data *self, float dt) {
  for (uint32_t i = 0; i < self->amount; i++) {
    self->position[i].x += self->direction[i].x * dt;
    self->position[i].y += self->direction[i].y * dt;
  }
  for (uint32_t i = 0; i < self->amount; i++) {
    if (self->position[i].y + self->size[i].y * 0.5f > GAME_TOP ||
        self->position[i].y - self->size[i].y * 0.5f < GAME_BOTTOM) self->direction[i].y *= -1;
    if (self->position[i].x + self->size[i].x * 0.5f > GAME_RIGHT ||
        self->position[i].x - self->size[i].x * 0.5f < GAME_LEFT) self->direction[i].x *= -1;
  }
}

void
something_render(struct something_data *self) {
  renderer_request_quads(self->amount, self->position, self->size, self->texture_position, self->texture_size, self->color, self->depth);
}
