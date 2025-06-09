#if DEV
#  include <assert.h>
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "yard/core.h"
#include "yard/math.h"
#include "yard/string.h"
#include "yard/shaders.h"

struct color { float r, g, b; };

struct vertex {
  struct v2    position;
  struct v2    texcoord;
  struct color blendcol;
  float        depth;
};

struct quad {
  struct vertex v[4];
};

#define QUAD_CAPACITY 10000
#define INDEX_CAPACITY (QUAD_CAPACITY*6)
#define VERTEX_CAPACITY (QUAD_CAPACITY*4)

struct renderer {
  struct quad quads[QUAD_CAPACITY];
  size_t quads_amount;
  uint32_t sh_default;
  uint32_t vao, vbo, ibo;
};

static struct renderer renderer;

#define SHADER_LOG_CAPACITY 512

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
      log_errorlf("%s: unknown shader type", __func__);
#endif
      return 0;
    } break;
  }
  uint32_t shader = glCreateShader(type);
  if (!shader) {
    log_errorlf("%s: couldn't make shader", __func__);
    return 0;
  }
#if DEV
  const int len = src->length;
  glShaderSource(shader, 1, &src->data, &len);
  glCompileShader(shader);
  int status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (!status) {
    int log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > SHADER_LOG_CAPACITY) {
      log_warnlf("%s: shader compile error will not be fully displayed. increase SHADER_LOG_CAPACITY", __func__);
    }
    char log[SHADER_LOG_CAPACITY];
    glGetShaderInfoLog(shader, SHADER_LOG_CAPACITY, 0, log);
    log_errorf("%s shader: %s", shader_str, log);
    glDeleteShader(shader);
    return 0;
  }
#endif
  return shader;
}

static uint32_t
shader_program_make(const struct str_view *vert_src, const struct str_view *frag_src) {
  log_infolf("%s: starting shader program creation...", __func__);
  uint32_t program = glCreateProgram();
  if (!program) {
    log_errorlf("%s: couldn't make shader program", __func__);
    return 0;
  }
  uint32_t vert = shader_make(GL_VERTEX_SHADER, vert_src);
  if (!vert) return 0;
  log_infolf("%s: compiled vertex shader", __func__);
  uint32_t frag = shader_make(GL_FRAGMENT_SHADER, frag_src);
  if (!frag) return 0;
  log_infolf("%s: compiled fragment shader", __func__);
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);
  glDeleteShader(vert);
  glDeleteShader(frag);
#if DEV
  int status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (!status) {
    int log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > SHADER_LOG_CAPACITY) {
      log_warnlf("%s: shader linking error will not be fully displayed. increase SHADER_LOG_CAPACITY", __func__);
    }
    char log[SHADER_LOG_CAPACITY];
    glGetProgramInfoLog(program, SHADER_LOG_CAPACITY, 0, log);
    log_errorf("shader program linking: %s", log);
    glDeleteProgram(program);
    return 0;
  }
#endif
  return program;
}

bool
renderer_make(struct arena *arena) {
  log_infol("making renderer...");
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  log_infol("loaded opengl functions");
  renderer.sh_default = shader_program_make(
    &SH_DEFAULT_VERT,
    &SH_DEFAULT_FRAG
  );
  if (!renderer.sh_default) return false;
  glUseProgram(renderer.sh_default);
  log_infol("created default shader");
  glEnable(GL_DEPTH_TEST);
  uint32_t *indices = arena_push_array(arena, true, uint32_t, INDEX_CAPACITY);
  if (!indices) {
    log_errorl("couldn't make indices buffer");
    return false;
  }
  uint32_t i = 0, j = 0;
  while (i < INDEX_CAPACITY) {
    indices[i++] = j + 0;
    indices[i++] = j + 1;
    indices[i++] = j + 2;
    indices[i++] = j + 2;
    indices[i++] = j + 3;
    indices[i++] = j + 0;
    j += 4;
  }
  struct color color = { 0.8f, 0.8f, 0.2f };
  renderer.quads[0].v[0].position = V2(-0.5f, -0.5f);
  renderer.quads[0].v[1].position = V2(+0.5f, -0.5f);
  renderer.quads[0].v[2].position = V2(+0.5f, +0.5f);
  renderer.quads[0].v[3].position = V2(-0.5f, +0.5f);
  renderer.quads[0].v[0].blendcol = color;
  renderer.quads[0].v[1].blendcol = color;
  renderer.quads[0].v[2].blendcol = color;
  renderer.quads[0].v[3].blendcol = color;
  glGenVertexArrays(1, &renderer.vao);
  glGenBuffers(1, &renderer.vbo);
  glGenBuffers(1, &renderer.ibo);
  glBindVertexArray(renderer.vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof (struct quad) * QUAD_CAPACITY, renderer.quads, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (uint32_t) * INDEX_CAPACITY, indices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, texcoord));
  glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, blendcol));
  glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, depth));
  log_infol("vao, vbo and ibo created successfully");
  log_infol("renderer creation complete!");
  return true;
}

bool
renderer_submit(void) {
  glClearColor(0.8f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  return true;
}
