#include "yard/window.h"
#include "yard/entities/player.h"

#define PLAYER_SPEED 0.1f

void
player_init(struct player_data *self) {
  self->position         = V2(0.0f, 0.0f);
  self->size             = V2(1.0f, 1.0f);
  self->texture_position = V2U(0, 0);
  self->texture_size     = V2U(16, 16);
}

void
player_update(struct player_data *self, float dt) {
  self->position.x += (window_is_key_down(K_RIGHT) - window_is_key_down(K_LEFT)) * dt * PLAYER_SPEED;
  self->position.y += (window_is_key_down(K_UP) - window_is_key_down(K_DOWN)) * dt * PLAYER_SPEED;
}

void
player_render(struct player_data *self) {
  renderer_request_quads(1,
    &self->position,
    &self->size,
    &self->texture_position,
    &self->texture_size,
    &WHITE,
    &(float) { 0.0f }
  );
}
