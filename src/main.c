#include "include/arena.h"
#include "include/window.h"
#include "include/renderer.h"
#include <glad/glad.h>

#define WINDOW_W 640
#define WINDOW_H 480

int
main(void) {
  if (!window_make(WINDOW_W, WINDOW_H)) return 1;
  if (!renderer_make()) return 1;
  struct arena *frame_arena = arena_make(0, 0);
  while (window_is_running()) {
    arena_clear(frame_arena);
  }
  return 0;
}
