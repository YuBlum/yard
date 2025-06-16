#include "engine/arena.h"
#include "game/entities.h"
#include "game/player.h"

struct entities {
  struct arena *arena;
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
  if (layout->has_player) {
    g_entities.player_data = arena_push_type(g_entities.arena, false, struct player_data);
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
  if (g_entities.player_data) player_render(g_entities.player_data);
}
