#include "yard/arena.h"
#include "yard/entities/entities.h"
#include "yard/entities/player.h"

// TODO: generate struct entities_layout, struct entities, entities_layout_set and maybe everything in this file with a script

struct entities {
  struct arena *arena;
  struct player_data *player_data;
  // struct foo_data *foo_data;
  // uint32_t foo_amount;
};

struct entities entities;

bool
entities_make(void) {
  entities.arena = arena_make(0, 0);
  if (!entities.arena) {
    log_errorl("couldn't make entities arena");
    return false;
  }
  return true;
}

bool
entities_layout_set(const struct entities_layout *layout) {
  if (!arena_clear(entities.arena)) { log_errorl("couldn't clear entities arena"); return false; }
  // entities.foo_data.array0 = arena_push_array(entities.arena, array0_type, layout->foo_amount);
  // if (!entities.foo_data.array0) { log and return }
  // entities.foo_data.array1 = arena_push_array(entities.arena, array1_type, layout->foo_amount);
  // if (!entities.foo_data.array1) { log and return }
  // ...
  // the player is a special case because you'll only have one of them:
  (void)layout;
  entities.player_data = arena_push_type(entities.arena, false, struct player_data);
  if (!entities.player_data) { log_errorl("couldn't allocate player data"); return false; }
  player_init(entities.player_data);
  return true;
}

void
entities_update(float dt) {
#if DEV
  if (!entities.arena) {
    log_errorlf("%s: arena is null", __func__);
    return;
  }
#endif
  if (entities.player_data) player_update(entities.player_data, dt);
}

void
entities_render(void) {
#if DEV
  if (!entities.arena) {
    log_errorlf("%s: arena is null", __func__);
    return;
  }
#endif
  if (entities.player_data) player_render(entities.player_data);
}
