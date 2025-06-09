#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "yard/core.h"
#include "yard/arena.h"
#include "yard/math.h"

struct color { float r, g, b; };
#define RGB(r, g, b) ((struct color) { r, g, b })
#define WHITE RGB(1.0, 1.0, 1.0)
#define BLACK RGB(0.0, 0.0, 0.0)

bool renderer_make(struct arena *arena);
void renderer_submit(void);

void renderer_quad(struct v2 position, struct v2 size, uint32_t texture_x, uint32_t texture_y, uint32_t texture_w, uint32_t texture_h, struct color color, float depth);

#endif/*__RENDERER_H__*/
