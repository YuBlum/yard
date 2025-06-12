#include "engine/arena.h"
#include "entities/entities.h"
#include "entities/something.h"
#include "entities/player.h"

struct entities {
  struct arena *arena;
  struct something_data something_data;
  struct player_data *player_data;
};

static struct entities g_entities;

bool
entities_make(void) {
  g_entities.arena = arena_make(0, 0);
  if (!g_entities.arena) {
    log_errorl("couldn't make entities arena");
    return false;
  }
  log_infol("entities manager creation completed!");
  return true;
}

bool
entities_layout_set(const struct entities_layout *layout) {
  if (!arena_clear(g_entities.arena)) { log_errorl("couldn't clear entities arena"); return false; }
  if (layout->something_amount) {
    g_entities.something_data.amount = layout->something_amount;
    g_entities.something_data.position = arena_push_array(g_entities.arena, false, struct v2, layout->something_amount);
    if (!g_entities.something_data.position) {
      log_errorl("couldn't allocate something position data");
      return false;
    }
    g_entities.something_data.size = arena_push_array(g_entities.arena, false, struct v2, layout->something_amount);
    if (!g_entities.something_data.size) {
      log_errorl("couldn't allocate something size data");
      return false;
    }
    g_entities.something_data.texture_position = arena_push_array(g_entities.arena, false, struct v2u, layout->something_amount);
    if (!g_entities.something_data.texture_position) {
      log_errorl("couldn't allocate something texture_position data");
      return false;
    }
    g_entities.something_data.texture_size = arena_push_array(g_entities.arena, false, struct v2u, layout->something_amount);
    if (!g_entities.something_data.texture_size) {
      log_errorl("couldn't allocate something texture_size data");
      return false;
    }
    g_entities.something_data.color = arena_push_array(g_entities.arena, false, struct color, layout->something_amount);
    if (!g_entities.something_data.color) {
      log_errorl("couldn't allocate something color data");
      return false;
    }
    g_entities.something_data.depth = arena_push_array(g_entities.arena, false, float, layout->something_amount);
    if (!g_entities.something_data.depth) {
      log_errorl("couldn't allocate something depth data");
      return false;
    }
    g_entities.something_data.direction = arena_push_array(g_entities.arena, false, struct v2, layout->something_amount);
    if (!g_entities.something_data.direction) {
      log_errorl("couldn't allocate something direction data");
      return false;
    }
    something_init(&g_entities.something_data);
  } else {
    g_entities.something_data.amount = 0;
  }
  if (layout->has_player) {
    g_entities.player_data = arena_push_type(g_entities.arena, false, uint32_t);
    if (!g_entities.player_data) {
      log_errorl("couldn't allocate player data");
      return false;
    }
    player_init(g_entities.player_data);
  } else {
    g_entities.player_data = 0;
  }
  return true;
}

void
entities_update(float dt) {
#if DEV
  if (!g_entities.arena) {
    log_errorlf("%s: arena is null", __func__);
    return;
  }
#endif
  if (g_entities.something_data.amount) something_update(&g_entities.something_data, dt);
  if (g_entities.player_data) player_update(g_entities.player_data, dt);
}

void
entities_render(void) {
#if DEV
  if (!g_entities.arena) {
    log_errorlf("%s: arena is null", __func__);
    return;
  }
#endif
  if (g_entities.something_data.amount) something_render(&g_entities.something_data);
  if (g_entities.player_data) player_render(g_entities.player_data);
}
