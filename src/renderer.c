#if DEV
#  include <assert.h>
#endif
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "yard/core.h"
#include "yard/string.h"
#include "yard/shaders.h"
#include "yard/renderer.h"

struct vertex {
  struct v2    position;
  struct v2    texcoord;
  struct color blendcol;
};

#define QUAD_CAPACITY 10000
#define INDEX_CAPACITY (QUAD_CAPACITY*6)
#define VERTEX_CAPACITY (QUAD_CAPACITY*4)

struct quad_vertices {
  struct vertex v[4];
};

struct quad_indices {
  uint32_t i[6];
};

struct renderer {
  struct quad_vertices vertices[QUAD_CAPACITY];
  struct quad_indices indices[QUAD_CAPACITY];
  float indices_depth[QUAD_CAPACITY];
  uint32_t quads_amount;
  uint32_t sh_default;
  int32_t  sh_default_proj;
  uint32_t texture_atlas;
};

#define LEFT   (-GAME_W * 0.5f)
#define RIGHT  (+GAME_W * 0.5f)
#define BOTTOM (-GAME_H * 0.5f)
#define TOP    (+GAME_H * 0.5f)

static struct renderer renderer;
static float projection[3*3] = {
  +2.0f/(RIGHT-LEFT)        , +0.0f                     , +0.0f,
  +0.0f                     , +2.0f/(TOP-BOTTOM)        , +0.0f,
  -(RIGHT+LEFT)/(RIGHT-LEFT), -(TOP+BOTTOM)/(TOP-BOTTOM), +1.0f,
};

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
      return 0;
    } break;
  }
#endif
  uint32_t shader = glCreateShader(type);
  if (!shader) {
    log_errorlf("%s: couldn't make shader", __func__);
    return 0;
  }
  const int len = src->length;
  glShaderSource(shader, 1, &src->data, &len);
  glCompileShader(shader);
#if DEV
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
renderer_make(void) {
  log_infol("making renderer...");
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  log_infol("loaded opengl functions");
  renderer.sh_default = shader_program_make(
    &SH_DEFAULT_VERT,
    &SH_DEFAULT_FRAG
  );
  if (!renderer.sh_default) return false;
  renderer.sh_default_proj = glGetUniformLocation(renderer.sh_default, "u_proj");
#if DEV
  if (renderer.sh_default_proj < 0) {
    log_errorl("couldn't get 'u_proj' location from default shader");
    return false;
  }
#endif
  glUseProgram(renderer.sh_default);
  glUniformMatrix3fv(renderer.sh_default_proj, 1, false, projection);
  log_infol("created default shader");
  int texture_atlas_width, texture_atlas_height, texture_atlas_channels;
  uint8_t *texture_atlas_data = stbi_load(
    "assets/atlas.png",
    &texture_atlas_width,
    &texture_atlas_height,
    &texture_atlas_channels,
    4
  );
#ifdef DEV
  if (!texture_atlas_data) {
    log_errorlf("couldn't load texture atlas");
    return false;
  }
  if (texture_atlas_width != ATLAS_SIZE) {
    log_errorlf("texture atlas width (%d) is not equal to ATLAS_SIZE (%u)", texture_atlas_width, ATLAS_SIZE);
    return false;
  }
  if (texture_atlas_height != ATLAS_SIZE) {
    log_errorlf("texture atlas height (%d) is not equal to ATLAS_SIZE (%u)", texture_atlas_height, ATLAS_SIZE);
    return false;
  }
#endif
  glGenTextures(1, &renderer.texture_atlas);
  glBindTexture(GL_TEXTURE_2D, renderer.texture_atlas);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_atlas_data);
  stbi_image_free(texture_atlas_data);
  log_infol("loaded texture atlas");
  uint32_t vao, vbo, ibo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ibo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof (renderer.vertices), 0, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (renderer.indices), 0, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, texcoord));
  glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, blendcol));
  log_infol("vao, vbo and ibo created successfully");
  log_infol("renderer creation complete!");
  return true;
}

static int
compare_indices(const void *r0, const void *r1) {
  float d0 = renderer.indices_depth[(size_t)((const struct quad_indices *)r0 - renderer.indices)];
  float d1 = renderer.indices_depth[(size_t)((const struct quad_indices *)r1 - renderer.indices)];
  if (d1 < d0) return -1;
  if (d1 > d0) return +1;
  return 0;
}

void
renderer_submit(void) {
  // NOTE: maybe instead of sorting and resetting the batch every frame you can have 'permanent' quads
  qsort(renderer.indices,  renderer.quads_amount, sizeof (struct quad_indices), compare_indices);
  glClearColor(0.8f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glBufferSubData(GL_ARRAY_BUFFER, 0, renderer.quads_amount * sizeof (struct quad_vertices), renderer.vertices);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, renderer.quads_amount * sizeof (struct quad_indices), renderer.indices);
  glDrawElements(GL_TRIANGLES, renderer.quads_amount * 6, GL_UNSIGNED_INT, 0);
  renderer.quads_amount = 0;
}

void
renderer_request_quads(uint32_t amount, const struct v2 positions[amount], const struct v2 sizes[amount], const struct v2u texture_positions[amount], const struct v2u texture_sizes[amount], const struct color colors[amount], float depths[amount]) {
#if DEV
  if (renderer.quads_amount + amount >= QUAD_CAPACITY) {
    log_warnlf("%s: trying to request to much quads to rendering. increase QUAD_CAPACITY", __func__);
    return;
  }
#endif
  struct v2 size_half;
  struct v2 tpos;
  struct v2 tsiz;
  for (uint32_t i = 0; i < amount; i++) {
    size_half = v2_muls(sizes[i], 0.5f);
    tpos = v2_muls(V2U_V2(texture_positions[i]), ATLAS_PIXEL);
    tsiz = v2_muls(V2U_V2(texture_sizes[i]), ATLAS_PIXEL);
    renderer.vertices[renderer.quads_amount + i].v[0].position = v2_add(positions[i], V2(-size_half.x, -size_half.y));
    renderer.vertices[renderer.quads_amount + i].v[1].position = v2_add(positions[i], V2(+size_half.x, -size_half.y));
    renderer.vertices[renderer.quads_amount + i].v[2].position = v2_add(positions[i], V2(+size_half.x, +size_half.y));
    renderer.vertices[renderer.quads_amount + i].v[3].position = v2_add(positions[i], V2(-size_half.x, +size_half.y));
    renderer.vertices[renderer.quads_amount + i].v[0].texcoord = v2_add(tpos, V2(0.0f  , tsiz.y));
    renderer.vertices[renderer.quads_amount + i].v[1].texcoord = v2_add(tpos, V2(tsiz.x, tsiz.y));
    renderer.vertices[renderer.quads_amount + i].v[2].texcoord = v2_add(tpos, V2(tsiz.x, 0.0f  ));
    renderer.vertices[renderer.quads_amount + i].v[3].texcoord = v2_add(tpos, V2(0.0f  , 0.0f  ));
    renderer.vertices[renderer.quads_amount + i].v[0].blendcol = colors[i];
    renderer.vertices[renderer.quads_amount + i].v[1].blendcol = colors[i];
    renderer.vertices[renderer.quads_amount + i].v[2].blendcol = colors[i];
    renderer.vertices[renderer.quads_amount + i].v[3].blendcol = colors[i];
  }
  for (uint32_t i = 0; i < amount; i++) renderer.indices_depth[renderer.quads_amount + i] = depths[i];
  for (uint32_t i = renderer.quads_amount; i < renderer.quads_amount + amount; i++) {
    renderer.indices[i].i[0] = i * 4 + 0;
    renderer.indices[i].i[1] = i * 4 + 1;
    renderer.indices[i].i[2] = i * 4 + 2;
    renderer.indices[i].i[3] = i * 4 + 2;
    renderer.indices[i].i[4] = i * 4 + 3;
    renderer.indices[i].i[5] = i * 4 + 0;
  }
  renderer.quads_amount += amount;
}
