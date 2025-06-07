#ifndef __ARENA_H__
#define __ARENA_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct arena;

struct arena_state {
  size_t position;
  size_t alignment;
  uint8_t *base;
};

struct arena *arena_make(size_t capacity, size_t alignment);
bool arena_destroy(struct arena *arena);

bool arena_set_alignment(struct arena *arena, size_t alignment);

void *arena_push(struct arena *arena, bool not_zeroed, size_t length);
bool arena_pop(struct arena *arena, size_t length);

bool arena_scratch_begin(struct arena *arena, size_t alignment, struct arena_state *state);
bool arena_scratch_end(struct arena *arena, struct arena_state *state);

#endif/*__ARENA_H__*/
