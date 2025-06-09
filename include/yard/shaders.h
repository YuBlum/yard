#ifndef __SHADERS_H__
#define __SHADERS_H__

#define SH_DEFAULT_VERT str_view_make_from_lit( \
"#version 460 core\n" \
"layout (location=0) in vec2  a_position;\n" \
"layout (location=1) in vec2  a_texcoord;\n" \
"layout (location=2) in vec3  a_blendcol;\n" \
"layout (location=3) in float a_depth;\n" \
"\n" \
"out vec2 v_texcoord;\n" \
"out vec3 v_blendcol;\n" \
"\n" \
"void\n" \
"main() {\n" \
"  gl_Position = vec4(a_position, a_depth, 1.0);\n" \
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

#endif/*__SHADERS_H__*/
