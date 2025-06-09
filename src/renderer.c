#if DEV
#  include <assert.h>
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "yard/core.h"
#include "yard/arena.h"
#include "yard/math.h"
#include "yard/string.h"

struct color { float r, g, b; };

struct vertex {
  struct v2    position;
  struct v2    texcoord;
  struct color blendcol;
};

struct quad {
  struct vertex v[4];
};

struct renderer {
  struct arena *quad_arena;
  struct quad *quads;
  size_t quads_amount;
  uint32_t sh_default;
};

static struct renderer renderer;

static bool
renderer_make_quads_buffer(void) {
  renderer.quad_arena = arena_make_typed(0, struct quad);
  if (!renderer.quad_arena) {
    log_error("couldn't create renderer quad arena");
    return false;
  }
  renderer.quads = arena_get_base(renderer.quad_arena);
  renderer.quads_amount = 0;
  return true;
}

#define SHADER_LOG_CAPACITY 4096

static uint32_t
shader_make(GLenum type, const struct str_view *src) {
#if DEV
  const char *shader_str = 0;
  switch (type) {
    case GL_VERTEX_SHADER: {
      shader_str = "vertex";
    } break;
    case GL_FRAGMENT_SHADER: {
      shader_str = "fragment";
    } break;
    default: {
      log_errorf("%s: unknown shader type", __func__);
#endif
      return 0;
    } break;
  }
  uint32_t shader = glCreateShader(type);
  if (!shader) {
    log_errorf("%s: couldn't create shader", __func__);
    return 0;
  }
#if DEV
  const int len = src->length;
  glShaderSource(shader, 1, &src->data, &len);
  glCompileShader(shader);
  int status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (!status) {
    char log[SHADER_LOG_CAPACITY];
    glGetShaderInfoLog(shader, SHADER_LOG_CAPACITY, 0, log);
    log_errorf("%s shader: %s", shader_str, log);
    glDeleteShader(shader);
    return 0;
  }
#endif
  return shader;
}

static inline uint32_t
shader_program_make(const struct str_view *vert_src, const struct str_view *frag_src) {
  log_infof("%s: starting shader program creation...", __func__);
  uint32_t program = glCreateProgram();
  if (!program) {
    log_errorf("%s: couldn't create shader program", __func__);
    return 0;
  }
  uint32_t vert = shader_make(GL_VERTEX_SHADER, vert_src);
  if (!vert) return 0;
  log_infof("%s: compiled vertex shader", __func__);
  uint32_t frag = shader_make(GL_FRAGMENT_SHADER, frag_src);
  if (!frag) return 0;
  log_infof("%s: compiled fragment shader", __func__);
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);
  glDeleteShader(vert);
  glDeleteShader(frag);
#if DEV
  int status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (!status) {
    char log[SHADER_LOG_CAPACITY];
    glGetProgramInfoLog(program, SHADER_LOG_CAPACITY, 0, log);
    log_errorf("shader program linking: %s", log);
    glDeleteProgram(program);
    return 0;
  }
#endif
  return program;
}

#define SH_DEFAULT_VERT str_view_make_from_lit( \
"#version 460 core\n" \
"layout (location=0) in vec2 a_position;\n" \
"layout (location=1) in vec2 a_texcoord;\n" \
"layout (location=2) in vec3 a_blendcol;\n" \
"\n" \
"out vec2 v_texcoord;\n" \
"out vec3 v_blendcol;\n" \
"\n" \
"void\n" \
"main() {\n" \
"  gl_Position = vec4(a_position, 0.0, 1.0);\n" \
"  v_texcoord = a_texcoord;\n" \
"  v_blendcol = a_blendcol;\n" \
"}\n")

#define SH_DEFAULT_FRAG str_view_make_from_lit( \
"#version 460 core\n" \
"in vec2 v_texcoord;\n" \
"in vec3 v_blendcol;\n" \
"\n" \
"out vec4 f_color;\n" \
"\n" \
"void\n" \
"main() {\n" \
"  f_color = vec4(v_blendcol, 1.0);\n" \
"}\n")

bool
renderer_make(void) {
  log_info("making renderer...");
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  log_info("loaded opengl functions");
  if (!renderer_make_quads_buffer()) return false;
  log_info("created quads buffer");
  renderer.sh_default = shader_program_make(
    &SH_DEFAULT_VERT,
    &SH_DEFAULT_FRAG
  );
  if (!renderer.sh_default) return false;
  log_info("created default shader");
  log_info("renderer creation complete!");
  return true;
}
