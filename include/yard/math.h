#ifndef __MATH_H__
#define __MATH_H__

#include <math.h>

struct v2 { float x, y; };
#define V2(x, y) (struct v2) {x, y}

static inline struct v2 v2_add(struct v2 v0, struct v2 v1) { return (struct v2) { v0.x+v1.x, v0.y+v1.y }; }
static inline struct v2 v2_sub(struct v2 v0, struct v2 v1) { return (struct v2) { v0.x-v1.x, v0.y-v1.y }; }
static inline struct v2 v2_mul(struct v2 v0, struct v2 v1) { return (struct v2) { v0.x*v1.x, v0.y*v1.y }; }
static inline struct v2 v2_div(struct v2 v0, struct v2 v1) { return (struct v2) { v0.x/v1.x, v0.y/v1.y }; }

#endif/*__MATH_H__*/
