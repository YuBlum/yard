#if DEV
#  include <assert.h>
#endif
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
  int32_t  sh_default_proj;
  uint32_t texture_atlas;
};

#define LEFT   (-GAME_W * 0.5f)
#define RIGHT  (+GAME_W * 0.5f)
#define BOTTOM (-GAME_H * 0.5f)
#define TOP    (+GAME_H * 0.5f)
#define FAR    1000.0f
#define NEAR   0.0f

static struct renderer renderer;
static float projection[4*4] = {
  +2.0f/(RIGHT-LEFT)        , +0.0f                     , +0.0f                 , +0.0f,
  +0.0f                     , +2.0f/(TOP-BOTTOM)        , +0.0f                 , +0.0f,
  +0.0f                     , +0.0f                     , +2.0f/(FAR-NEAR)      , +0.0f,
  -(RIGHT+LEFT)/(RIGHT-LEFT), -(TOP+BOTTOM)/(TOP-BOTTOM), -(FAR+NEAR)/(FAR-NEAR), +1.0f,
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

#define ATLAS_SIZE 512
#define ATLAS_PIXEL (1.0/ATLAS_SIZE)

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
  renderer.sh_default_proj = glGetUniformLocation(renderer.sh_default, "u_proj");
#if DEV
  if (renderer.sh_default_proj < 0) {
    log_errorl("couldn't get 'u_proj' location from default shader");
    return false;
  }
#endif
  glUseProgram(renderer.sh_default);
  glUniformMatrix4fv(renderer.sh_default_proj, 1, false, projection);
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
  log_warnl("actually implement the texture atlas from texture atlas data");
  log_infol("loaded texture atlas");
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
  uint32_t vao, vbo, ibo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ibo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof (struct quad) * QUAD_CAPACITY, 0, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (uint32_t) * INDEX_CAPACITY, indices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, texcoord));
  glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, blendcol));
  glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof (struct vertex), (void *)offsetof (struct vertex, depth));
  log_infol("vao, vbo and ibo created successfully");
  log_infol("renderer creation complete!");
  return true;
}

void
renderer_submit(void) {
  glClearColor(0.8f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glBufferSubData(GL_ARRAY_BUFFER, 0, renderer.quads_amount * sizeof (struct quad), renderer.quads);
  glDrawElements(GL_TRIANGLES, renderer.quads_amount*6, GL_UNSIGNED_INT, 0);
  renderer.quads_amount = 0;
}

void
renderer_quad(struct v2 position, struct v2 size, uint32_t texture_x, uint32_t texture_y, uint32_t texture_w, uint32_t texture_h, struct color color, float depth) {
#if DEV
  if (renderer.quads_amount >= QUAD_CAPACITY) {
    log_warnlf("%s: quads ran out of memory. increase QUAD_CAPACITY", __func__);
    return;
  }
#endif
  if (depth < 0) depth = 0;
  struct quad *quad = &renderer.quads[renderer.quads_amount++];
  size = v2_mul(size, V2(0.5, 0.5));
  struct v2 texture_pos = { texture_x * ATLAS_PIXEL, texture_y * ATLAS_PIXEL };
  struct v2 texture_dim = { texture_w * ATLAS_PIXEL, texture_h * ATLAS_PIXEL };
  quad->v[0].position = V2(position.x - size.x, position.y - size.y);
  quad->v[1].position = V2(position.x + size.x, position.y - size.y);
  quad->v[2].position = V2(position.x + size.x, position.y + size.y);
  quad->v[3].position = V2(position.x - size.x, position.y + size.y);
  quad->v[0].texcoord = V2(texture_pos.x                , texture_pos.y + texture_dim.y);
  quad->v[1].texcoord = V2(texture_pos.x + texture_dim.x, texture_pos.y + texture_dim.y);
  quad->v[2].texcoord = V2(texture_pos.x + texture_dim.x, texture_pos.y                );
  quad->v[3].texcoord = V2(texture_pos.x                , texture_pos.y                );
  quad->v[0].blendcol = color;
  quad->v[1].blendcol = color;
  quad->v[2].blendcol = color;
  quad->v[3].blendcol = color;
  quad->v[0].depth = depth;
  quad->v[1].depth = depth;
  quad->v[2].depth = depth;
  quad->v[3].depth = depth;
}

// TODO: texture atlas and texturing
