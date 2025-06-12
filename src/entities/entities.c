#include "yard/arena.h"
#include "yard/entities/entities.h"
#include "yard/entities/something.h"
#include "yard/entities/player.h"

struct entities {
  struct arena *arena;
  struct something_data something_data;
  struct player_data *player_data;
};

static struct entities entities;

bool
entities_make(void) {
  entities.arena = arena_make(0, 0);
  if (!entities.arena) {
    log_errorl("couldn't make entities arena");
    return false;
  }
  log_infol("entities manager creation completed!");
  return true;
}

bool
entities_layout_set(const struct entities_layout *layout) {
  if (!arena_clear(entities.arena)) { log_errorl("couldn't clear entities arena"); return false; }
  if (layout->something_amount) {
    entities.something_data.amount = layout->something_amount;
    entities.something_data.position = arena_push_array(entities.arena, false, struct v2, layout->something_amount);
    if (!entities.something_data.position) {
      log_errorl("couldn't allocate something position data");
      return false;
    }
    entities.something_data.size = arena_push_array(entities.arena, false, struct v2, layout->something_amount);
    if (!entities.something_data.size) {
      log_errorl("couldn't allocate something size data");
      return false;
    }
    entities.something_data.texture_position = arena_push_array(entities.arena, false, struct v2u, layout->something_amount);
    if (!entities.something_data.texture_position) {
      log_errorl("couldn't allocate something texture_position data");
      return false;
    }
    entities.something_data.texture_size = arena_push_array(entities.arena, false, struct v2u, layout->something_amount);
    if (!entities.something_data.texture_size) {
      log_errorl("couldn't allocate something texture_size data");
      return false;
    }
    entities.something_data.color = arena_push_array(entities.arena, false, struct color, layout->something_amount);
    if (!entities.something_data.color) {
      log_errorl("couldn't allocate something color data");
      return false;
    }
    entities.something_data.depth = arena_push_array(entities.arena, false, float, layout->something_amount);
    if (!entities.something_data.depth) {
      log_errorl("couldn't allocate something depth data");
      return false;
    }
    entities.something_data.direction = arena_push_array(entities.arena, false, struct v2, layout->something_amount);
    if (!entities.something_data.direction) {
      log_errorl("couldn't allocate something direction data");
      return false;
    }
    something_init(&entities.something_data);
  } else {
    entities.something_data.amount = 0;
  }
  if (layout->has_player) {
    entities.player_data = arena_push_type(entities.arena, false, uint32_t);
    if (!entities.player_data) {
      log_errorl("couldn't allocate player data");
      return false;
    }
    player_init(entities.player_data);
  } else {
    entities.player_data = 0;
  }
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
  if (entities.something_data.amount) something_update(&entities.something_data, dt);
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
  if (entities.something_data.amount) something_render(&entities.something_data);
  if (entities.player_data) player_render(entities.player_data);
}
