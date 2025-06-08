#include <glad/glad.h>
#include "yard/core.h"
#include "yard/arena.h"
#include "yard/math.h"
#include "yard/string.h"

struct color { float r, g, b, a; };

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

static bool
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

#if 0
static uint32_t
shader_make(GLenum type, struct string_view src) {
  const char *shader_str = 0;
  switch (type) {
    case GL_VERTEX_SHADER: {
      shader_str = "vertex";
    } break;
    case GL_FRAGMENT_SHADER: {
      shader_str = "fragment";
    } break;
    default: {
#if DEV
      log_error("shader_make: unknown shader type");
#endif
      return 0;
    } break;
  }
  uint32_t shader = glCreateShader(type);
  if (!shader) {
#if DEV
    log_error("shader_make: couldn't create shader");
#endif
    return 0;
  }
  return shader;
}
#endif

bool
renderer_make(void) {
  log_info("making renderer...");
  if (!renderer_make_quads_buffer()) return false;
  log_info("created quads buffer");
  log_info("renderer creation complete!");
  return true;
}
