#include <stdlib.h>
#include <time.h>
#include "engine/arena.h"
#include "engine/window.h"
#include "engine/mixer.h"
#include "engine/renderer.h"
#include "game/entities.h"
#include "game/player.h"

#define WINDOW_W (GAME_W_PIXEL * GAME_S)
#define WINDOW_H (GAME_H_PIXEL * GAME_S)

int
main(void) {
  srand(time(0));
  struct arena *tmp_arena = arena_make(0, 0);
  if (!tmp_arena) {
    log_errorl("couldn't make main temporary arena");
    return 1;
  }
  if (!window_make(WINDOW_W, WINDOW_H)) return 1;
  if (!mixer_make()) {
    window_destroy();
    return 1;
  }
  if (!renderer_make()) {
    mixer_destroy();
    window_destroy();
    return 1;
  }
  if (!entities_make()) {
    mixer_destroy();
    window_destroy();
    return 1;
  }
  if (!entities_layout_set(&(struct entities_layout) {
    .has_player = true,
    .something_amount = 1000,
  })) {
    mixer_destroy();
    window_destroy();
    return 1;
  } // TODO: remove this from here
  uint32_t music = mixer_sound_reserve("assets/thejester.wav", true, true).sound_handle; // TODO: remove this from here
  uint32_t death_sound = mixer_sound_reserve("assets/vgdeathsound.wav", false, false).sound_handle; // TODO: remove this from here
  uint32_t menu_sound = mixer_sound_reserve("assets/vgmenuselect.wav", false, false).sound_handle; // TODO: remove this from here
  while (window_is_running()) {
    if (window_is_key_down(K_EXIT)) window_close();
    entities_update(window_get_delta_time());
    if (window_is_key_press(K_A)) (void)mixer_sound_play(death_sound);
    if (window_is_key_press(K_B)) (void)mixer_sound_play(menu_sound);
    if (window_is_key_down(K_RIGHT)) {
      if (window_is_key_press(K_UP)) (void)mixer_inc_volume(0.05f);
      if (window_is_key_press(K_DOWN)) (void)mixer_dec_volume(0.05f);
      if (window_is_key_press(K_LEFT)) (void)mixer_set_volume(0.5f);
    } else {
      if (window_is_key_press(K_UP)) (void)mixer_sound_inc_volume(music, 0.05f);
      if (window_is_key_press(K_DOWN)) (void)mixer_sound_dec_volume(music, 0.05f);
      if (window_is_key_press(K_LEFT)) (void)mixer_sound_set_volume(music, 0.5f);
    }
    entities_render();
    renderer_submit();
    arena_clear(tmp_arena);
  }
  mixer_destroy();
  window_destroy();
  return 0;
}
