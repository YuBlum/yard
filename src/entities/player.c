#include "yard/entities/player.h"
#include "yard/window.h"

#define SPEED 5.0f

void
player_init(struct player_data *self) {
  self->position = V2(0.0f, 0.0f);
  self->texture_position = V2U(16, 16);
}

void
player_update(struct player_data *self, float dt) {
  self->position.x += (window_is_key_down(K_RIGHT) - window_is_key_down(K_LEFT)) * SPEED * dt;
  self->position.y += (window_is_key_down(K_UP) - window_is_key_down(K_DOWN)) * SPEED * dt;
}

void
player_render(struct player_data *self) {
  renderer_request_quads(1, &self->position, &V2(1.0f, 1.0f), &self->texture_position, &V2U(16, 16), &WHITE, &(float) { 1.0f });
}
