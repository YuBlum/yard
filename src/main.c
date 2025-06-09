#include "yard/arena.h"
#include "yard/window.h"
#include "yard/renderer.h"

#define WINDOW_W 640
#define WINDOW_H 480

int
main(void) {
  if (!window_make(WINDOW_W, WINDOW_H)) return 1;
  if (!renderer_make()) return 1;
  struct arena *frame_arena = arena_make(0, 0);
  while (window_is_running()) {
    if (window_is_key_down(K_EXIT)) window_close();
    arena_clear(frame_arena);
  }
  return 0;
}
