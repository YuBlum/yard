#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "yard/core.h"
#include "yard/arena.h"

bool renderer_make(struct arena *arena);
bool renderer_submit(void);

#endif/*__RENDERER_H__*/
