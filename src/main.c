#include "yard/arena.h"
#include "yard/window.h"
#include "yard/renderer.h"

#define WINDOW_W 640
#define WINDOW_H 480

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
  while (window_is_running()) {
    if (window_is_key_down(K_EXIT)) window_close();
    if (!renderer_submit()) {
      log_errorl("couldn't submit render calls");
    }
    arena_clear(tmp_arena);
  }
  return 0;
}
