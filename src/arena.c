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

#if DEV
/* NOTE: assumes non-zero alignment; if 0 is passed, ensure it is handled later */
static inline bool
is_power_of_2_non_zero(size_t alignment, const char *func_name) {
  if ((alignment & (alignment - 1)) != 0) {
    log_errorf("%s: 'alignment' has to be a power of 2", func_name);
    return false;
  }
  return true;
}
#endif

struct arena *
arena_make(size_t capacity, size_t alignment) {
  if (!capacity) capacity = 1ull << 33; // defaults to 8G
  struct arena *arena = mmap(0, sizeof (struct arena) + capacity, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
# if DEV
  if (arena == MAP_FAILED) {
    log_error("arena_make: couldn't allocate memory for the arena");
    return 0;
  }
  if (!is_power_of_2_non_zero(alignment, "arena_make")) return 0;
#endif
  arena->alignment = alignment ? alignment : sizeof (void *);
  arena->capacity = capacity;
  arena->position = 0;
  arena->top = 0;
  arena->base = arena->data;
  return arena;
}

bool
arena_destroy(struct arena *arena) {
#if DEV
  if (!arena) {
    log_error("arena_destroy: passing invalid arena");
    return false;
  }
  int munmap_res =
#endif
  munmap(arena, sizeof (struct arena) + arena->capacity);
#if DEV
  if (munmap_res != 0) {
    log_error("arena_destroy: munmap failed");
    return false;
  }
#endif
  return true;
}

void *
arena_get_base(struct arena *arena) {
#if DEV
  if (!arena) {
    log_error("arena_destroy: passing invalid arena");
    return false;
  }
#endif
  return arena->base;
}

static inline size_t
__arena_align__(size_t length, size_t alignment
#if DEV
    , const char *func_name
#endif
) {
#if DEV
  if (!alignment) {
    log_errorf("%s: passing 0 alignment", func_name);
    return length;
  }
#endif
  return (length + alignment - 1) & ~(alignment - 1);
}
#if DEV
#  define arena_align(length, alignment) __arena_align__(length, alignment, __func__)
#else
#  define arena_align __arena_align__
#endif

void *
arena_push(struct arena *arena, bool not_zeroed, size_t length) {
#if DEV
  if (!arena) {
    log_error("arena_push: passing invalid arena");
    return 0;
  }
  if ((uintptr_t)arena->base % arena->alignment != 0) log_warn("arena_push: poorly aligned arena");
#endif
  if (arena->top >= arena->capacity) {
    log_error("arena_push: arena ran out of memory");
    return 0;
  }
  length = arena_align(length, arena->alignment);
#if DEV
  if (length > arena->capacity - arena->position) {
    log_errorf("arena_push: arena does not have enough memory to allocate %zu bytes", length);
    return 0;
  }
#endif
  void *res = &arena->base[arena->position];
  arena->position += length;
  arena->top += length;
  if (!not_zeroed) memset(res, 0, length);
  return res;
}

bool
arena_pop(struct arena *arena, size_t length) {
#if DEV
  if (!arena) {
    log_error("arena_pop: passing invalid arena");
    return false;
  }
#endif
  length = arena_align(length, arena->alignment);
#if DEV
  if (length > arena->position) {
    log_errorf("arena_pop: length '%zu' is greater than the arena position '%zu'", length, arena->position);
    return false;
  }
#endif
  arena->position -= length;
  arena->top -= length;
  return true;
}

bool
arena_clear(struct arena *arena) {
#if DEV
  if (!arena) {
    log_error("arena_clear: passing invalid arena");
    return false;
  }
#endif
  arena->position = 0;
  arena->top = 0;
  return true;
}

bool
arena_scratch_begin(struct arena *arena, struct arena_state *state, size_t alignment) {
#if DEV
  if (!arena) {
    log_error("arena_scratch_begin: passing invalid arena");
    return false;
  }
  if (!state) {
    log_error("arena_scratch_begin: passing invalid state");
    return false;
  }
#endif
  if (arena->top >= arena->capacity) {
    log_error("arena_scratch_begin: arena ran out of memory");
    return false;
  }
  *state = (struct arena_state) {
    .position  = arena->position,
    .alignment = arena->alignment,
    .base      = arena->base,
    .top       = arena->top,
  };
#if DEV
  if (!is_power_of_2_non_zero(alignment, "arena_scratch_begin")) return false;
#endif
  if (!alignment) alignment = sizeof (void *);
  size_t new_top = arena_align(arena->top, alignment);
#if DEV
  if (new_top >= arena->capacity) {
    log_error("arena_scratch_begin: new alignment makes arena exceed capacity");
    return false;
  }
#endif
  arena->base = &arena->data[new_top];
  arena->top = new_top;
  arena->alignment = alignment;
  arena->position = 0;
  return true;
}

bool
arena_scratch_end(struct arena *arena, struct arena_state *state) {
#if DEV
  if (!arena) {
    log_error("arena_scratch_end: passing invalid arena");
    return false;
  }
  if (!state) {
    log_error("arena_scratch_end: passing invalid state");
    return false;
  }
#endif
  arena->alignment = state->alignment;
  arena->position = state->position;
  arena->base = state->base;
  arena->top = state->top;
  *state = (struct arena_state) { 0 };
  return true;
}
