#ifndef __ARENA_H__
#define __ARENA_H__

#include "engine/core.h"
#include <stdalign.h>

struct arena {
  size_t capacity;
  size_t top;
  size_t position;
  size_t position_prv;
  size_t alignment;
  uint32_t array_length; /* <- for dynamic arrays */
  uint8_t *base;
  uint8_t data[];
};

struct arena_state {
  size_t top;
  size_t position;
  size_t position_prv;
  size_t alignment;
  uint8_t *base;
};

struct arena *arena_make(size_t capacity, size_t alignment);
#define arena_make_typed(capacity, T) arena_make(capacity * sizeof (T), alignof (T))
bool arena_destroy(struct arena *arena);

void *arena_get_base(struct arena *arena);
bool arena_is_last_alloc(struct arena *arena, void *ptr);
size_t arena_last_alloc_real_length(struct arena *arena);

void *arena_push(struct arena *arena, bool not_zeroed, size_t length);
#define arena_push_type(arena, not_zeroed, T) arena_push(arena, not_zeroed, sizeof (T))
#define arena_push_array(arena, not_zeroed, T, amount) arena_push(arena, not_zeroed, sizeof (T) * (amount))
bool arena_pop(struct arena *arena, size_t length);
#define arena_pop_type(arena, T) arena_pop(arena, sizeof (T))
#define arena_pop_array(arena, T, amount) arena_pop(arena, sizeof (T) * (amount))
bool arena_clear(struct arena *arena);

bool arena_scratch_begin(struct arena *arena, struct arena_state *state, size_t alignment);
#define arena_scratch_begin_typed(arena, state, T) arena_scratch_begin(arena, state, alignof (T))
bool arena_scratch_end(struct arena *arena, struct arena_state *state);

void *_arena_array_make(size_t capacity, size_t alignment);

static inline void *
__arena_array_make__(size_t capacity, size_t alignment) {
  struct arena *arena = arena_make(capacity, alignment);
  if (!arena) return 0;
  return arena ? arena + 1 : 0;
}
static inline void *
__arena_array_grow__(struct arena *arena, uint32_t type_size, uint32_t amount) {
  void *res = arena_push(arena, true, type_size * amount);
  if (res) arena->array_length += amount;
  return res;
}
#define arena_array_make(capacity, T) ((T *)__arena_array_make__(capacity * sizeof (T), alignof (T)))
#define arena_array_get_arena(array) (((struct arena *)(array)) - 1)
#define arena_array_grow(array, amount) ((typeof (array[0]) *)__arena_array_grow__(arena_array_get_arena(array), sizeof (array[0]), amount))
#define arena_array_push(array, value) do { \
  typeof(array[0]) *p = arena_array_grow(array, 1); \
  if (p) { \
    *p = value; \
  } \
} while (0)
#define arena_array_pop(array) do { \
  struct arena *arena = arena_array_get_arena(array); \
  if (arena_pop_type(arena, typeof (array[0]))) { \
    arena->array_length--;\
  } \
} while (0)
#define arena_array_length(array) (arena_array_get_arena(array)->array_length)
#define arena_array_clear(array) do { \
  struct arena *arena = arena_array_get_arena(array); \
  if (arena_clear(arena)) { \
    arena->array_length = 0;\
  } \
} while (0)
#define arena_array_destroy(array) arena_destroy(arena_array_get_arena(array))

#endif/*__ARENA_H__*/
