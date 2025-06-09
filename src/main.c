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
  if (!renderer_make(tmp_arena)) return 1;
  arena_clear(tmp_arena);
  struct v2 p = { 0.0f, 0.0f };
  float p_depth = 0.0f;
  while (window_is_running()) {
    if (window_is_key_down(K_EXIT)) window_close();
    p.x += (window_is_key_down(K_RIGHT) - window_is_key_down(K_LEFT)) * 0.1f;
    p.y += (window_is_key_down(K_UP)    - window_is_key_down(K_DOWN)) * 0.1f;
    p_depth += (window_is_key_press(K_B) - window_is_key_press(K_A));
    log_infolf("p_depth: %f", p_depth);
    renderer_quad(p, V2(1.0f, 1.0f), RGB(0.8f, 0.8f, 0.2f), p_depth);
    renderer_quad(V2(0.5f, 0.5f), V2(1.0f, 1.0f), RGB(0.4f, 0.2f, 0.8f), 0.0f);
    renderer_submit();
    arena_clear(tmp_arena);
  }
  return 0;
}
