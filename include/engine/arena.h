#ifndef __ARENA_H__
#define __ARENA_H__

#include "engine/core.h"
#include <stdalign.h>

struct arena;

struct arena_state {
  size_t top;
  size_t position;
  size_t position_prv;
  size_t alignment;
  uint8_t *base;
};

struct arena *arena_make(size_t capacity, size_t alignment);
#define arena_make_typed(capacity, T) arena_make(capacity, alignof (T))
bool arena_destroy(struct arena *arena);

void *arena_get_base(struct arena *arena);
bool arena_is_last_alloc(struct arena *arena, void *ptr);
size_t arena_last_alloc_real_length(struct arena *arena);

void *arena_push(struct arena *arena, bool not_zeroed, size_t length);
#define arena_push_type(arena, not_zeroed, T) arena_push(arena, not_zeroed, sizeof (T))
#define arena_push_array(arena, not_zeroed, T, amount) arena_push(arena, not_zeroed, sizeof (T) * amount)
bool arena_pop(struct arena *arena, size_t length);
#define arena_pop_type(arena, T) arena_pop(arena, sizeof (T))
#define arena_pop_array(arena, T, amount) arena_pop(arena, sizeof (T) * amount)
bool arena_clear(struct arena *arena);

bool arena_scratch_begin(struct arena *arena, struct arena_state *state, size_t alignment);
#define arena_scratch_begin_typed(arena, state, T) arena_scratch_begin(arena, state, alignof (T))
bool arena_scratch_end(struct arena *arena, struct arena_state *state);

#endif/*__ARENA_H__*/
