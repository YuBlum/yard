#include <sys/mman.h>
#include <string.h>
#include "include/core.h"
#include "include/arena.h"

struct arena {
  size_t capacity;
  size_t position;
  size_t alignment;
  size_t top;
  uint8_t *base;
  uint8_t data[];
};

struct arena *
arena_make(size_t capacity, size_t alignment) {
  if (!capacity) capacity = 1ull << 33; // defaults to 8G
  struct arena *arena = mmap(0, sizeof (struct arena) + capacity, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (!arena) {
    log_warn("arena_make: couldn't allocate memory for the arena");
    return 0;
  }
  arena->alignment = alignment ? alignment : sizeof (void *);
  arena->capacity = capacity;
  arena->position = 0;
  arena->top = 0;
  arena->base = arena->data;
  return arena;
}

bool
arena_destroy(struct arena *arena) {
  if (!arena) {
    log_warn("arena_destroy: passing invalid arena");
    return false;
  }
  munmap(arena, sizeof (struct arena) + arena->capacity);
  return true;
}

bool
arena_set_alignment(struct arena *arena, size_t alignment) {
  if (!arena) {
    log_warn("arena_set_alignment: passing invalid arena");
    return false;
  }
  arena->alignment = alignment ? alignment : sizeof (void *);
  return true;
}

static size_t
align(size_t length, size_t alignment) {
  size_t remainder = length % alignment;
  if (remainder > 0) length += alignment - remainder;
  return length;
}

void *
arena_push(struct arena *arena, bool not_zeroed, size_t length) {
  if (!arena) {
    log_warn("arena_push: passing invalid arena");
    return false;
  }
  if (arena->top >= arena->capacity) {
    log_warn("arena_push: arena ran out of memory");
    return 0;
  }
  length = align(length, arena->alignment);
  if (length > arena->capacity - arena->position) {
    log_warnf("arena_push: arena does not have enough memory to allocate %zu bytes", length);
    return 0;
  }
  void *res = &arena->base[arena->position];
  arena->position += length;
  arena->top += length;
  if (not_zeroed) memset(res, 0, length);
  return res;
}

bool
arena_pop(struct arena *arena, size_t length) {
  if (!arena) {
    log_warn("arena_pop: passing invalid arena");
    return false;
  }
  length = align(length, arena->alignment);
  if (length > arena->position) {
    log_warnf("arena_pop: length '%zu' is greater than the arena position '%zu'", length, arena->position);
    return false;
  }
  arena->position -= length;
  arena->top -= length;
  return true;
}

bool
arena_scratch_begin(struct arena *arena, size_t alignment, struct arena_state *state) {
  if (!arena) {
    log_warn("arena_scratch_begin: passing invalid arena");
    return false;
  }
  if (arena->top >= arena->capacity) {
    log_warn("arena_scratch_begin: arena ran out of memory");
    return false;
  }
  *state = (struct arena_state) {
    .position  = arena->position,
    .alignment = arena->alignment,
    .base      = arena->base,
  };
  arena->base = &arena->base[arena->position];
  arena->position = 0;
  arena->alignment = alignment;
  return true;
}

bool
arena_scratch_end(struct arena *arena, struct arena_state *state) {
  if (!arena) {
    log_warn("arena_scratch_end: passing invalid arena");
    return false;
  }
  arena->alignment = state->alignment;
  arena->position = state->position;
  arena->base = state->base;
  *state = (struct arena_state) { 0 };
  return true;
}
