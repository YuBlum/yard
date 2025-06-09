#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "yard/core.h"
#include "yard/arena.h"
#include "yard/math.h"

struct color { float r, g, b; };
#define RGB(r, g, b) ((struct color) { r, g, b })

bool renderer_make(struct arena *arena);
void renderer_submit(void);

void renderer_quad(struct v2 position, struct v2 size, struct color color, float depth);

#endif/*__RENDERER_H__*/
