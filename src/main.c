#include "yard/arena.h"
#include "yard/window.h"
#include "yard/renderer.h"

#define WINDOW_W (GAME_W_PIXEL * GAME_S)
#define WINDOW_H (GAME_H_PIXEL * GAME_S)

int
main(void) {
  struct arena *tmp_arena = arena_make(0, 0);
  if (!tmp_arena) {
    log_errorl("couldn't make main temporary arena");
    return 1;
  }
  if (!window_make(WINDOW_W, WINDOW_H)) return 1;
  if (!renderer_make()) return 1;
  struct v2 pos[] = {
    { +0.0f, +0.0f },
    { +0.5f, +0.5f },
    { -2.0f, -2.0f },
  };
  struct v2 siz[] = {
    { 1.0f, 1.0f },
    { 1.0f, 1.0f },
    { 1.5f, 0.5f },
  };
  struct color col[] = {
    { 0.8f, 0.8f, 0.2f },
    { 0.4f, 0.2f, 0.8f },
    { 0.2f, 0.8f, 0.2f },
  };
  struct v2u tpos[] = {
    V2US(0),
    V2US(16),
    ATLAS_PIXEL_POS,
  };
  struct v2u tsiz[] = {
    V2US(16),
    V2US(16),
    V2US(1),
  };
  float depth[] = {
    +0.0f,
    +0.0f,
    -1.0f,
  };
  while (window_is_running()) {
    if (window_is_key_down(K_EXIT)) window_close();
    pos[0].x += (window_is_key_down(K_RIGHT) - window_is_key_down(K_LEFT)) * 0.1f;
    pos[0].y += (window_is_key_down(K_UP)    - window_is_key_down(K_DOWN)) * 0.1f;
    depth[0] += (window_is_key_press(K_B) - window_is_key_press(K_A));
    log_infolf("depth[0]: %f", depth[0]);
    renderer_request_quads(3, pos, siz, tpos, tsiz, col, depth);
    renderer_submit();
    arena_clear(tmp_arena);
  }
  return 0;
}
