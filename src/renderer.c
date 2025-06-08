#include "include/core.h"
#include "include/arena.h"
#include "include/math.h"

struct color { float r, g, b, a; };
#define COLOR(r, g, b, a) (struct color) {r, g, b, a}

struct vertex {
  struct v2 position;
  struct v2 texcoord;
};

struct quad {
  struct vertex v[4];
};

struct renderer {
  struct arena *quad_arena;
  struct quad *quads;
  size_t quads_amount;
};

static struct renderer renderer;

bool
renderer_make_quads_buffer(void) {
  renderer.quad_arena = arena_make_typed(0, struct quad);
  if (!renderer.quad_arena) {
    log_info("couldn't create renderer quad arena");
    return false;
  }
  renderer.quads = arena_get_base(renderer.quad_arena);
  renderer.quads_amount = 0;
  return true;
}

bool
renderer_make(void) {
  log_info("making renderer...");
  if (!renderer_make_quads_buffer()) return false;
  log_info("quads buffer created");
  return true;
}
