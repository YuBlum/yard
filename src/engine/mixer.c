#include <miniaudio.h>
#include <string.h>
#include "engine/core.h"
#include "engine/arena.h"
#include "engine/mixer.h"

struct sound {
  ma_decoder decoder;
  bool active;
};

struct mixer {
  ma_device device;
  struct arena *tmp_buffer_arena;
  struct sound *sounds;
};
#define TMP_BUFFER_ARENA_CAPACITY (1ul<<20)

static struct mixer g_mixer;

static void
data_callback(ma_device *device, void *output_buffer, const void *input_buffer, uint32_t frame_count) {
  (void)device;
  (void)input_buffer;
  float *tmp_buffer = arena_push_array(g_mixer.tmp_buffer_arena, true, float, TMP_BUFFER_ARENA_CAPACITY);
  if (!tmp_buffer) {
    log_errorl("couldn't allocate temporary buffer for mixing");
    return;
  }
  float *out = output_buffer;
  uint32_t sounds_amount = arena_array_length(g_mixer.sounds);
  for (uint32_t i = 0; i < sounds_amount; i++) {
    if (!g_mixer.sounds[i].active) continue;
    (void)memset(tmp_buffer, 0, frame_count * sizeof (float));
    ma_result res = ma_data_source_read_pcm_frames(&g_mixer.sounds[i].decoder, tmp_buffer, frame_count, 0);
    if (res != MA_SUCCESS) {
      g_mixer.sounds[i].active = false;
      continue;
    }
    for (uint32_t sample = 0; sample < frame_count; sample++) {
      out[sample] += tmp_buffer[sample];
      if (out[sample] > 1.0f) out[sample] = 1.0f;
      if (out[sample] < -1.0f) out[sample] = -1.0f;
    }
  }
  if (!arena_clear(g_mixer.tmp_buffer_arena)) {
    log_errorl("couldn't clear mixer's temporary buffer arena");
  }
}

bool
mixer_make(void) {
  log_infol("making sound mixer...");
  ma_device_config config = ma_device_config_init(ma_device_type_playback);
  config.playback.format   = ma_format_f32;
  config.playback.channels = 1;
  config.sampleRate        = 48000;
  config.dataCallback      = data_callback;
  if (ma_device_init(0, &config, &g_mixer.device) != MA_SUCCESS) {
    log_errorl("couldn't obtain device for the mixer");
    return false;
  }
  log_infol("obtained device for the mixer");
  g_mixer.tmp_buffer_arena = arena_make_typed(TMP_BUFFER_ARENA_CAPACITY, float);
  if (!g_mixer.tmp_buffer_arena) {
    ma_device_uninit(&g_mixer.device);
    log_errorl("couldn't make mixer temporary buffer arena");
    return false;
  }
  log_infol("mixer temporary buffer arena created");
  g_mixer.sounds = arena_array_make(0, struct sound);
  if (!g_mixer.sounds) {
    log_errorl("couldn't make mixer sounds buffer");
    return false;
  }
  log_infol("mixer sounds buffer created");
  if (ma_device_start(&g_mixer.device) != MA_SUCCESS) {
    ma_device_uninit(&g_mixer.device);
    log_errorl("couldn't start sound mixer");
  }
  log_infol("sound mixer started");
  log_infol("mixer creation complete!");
  return true;
}

void
mixer_destroy(void) {
  ma_device_uninit(&g_mixer.device);
}

struct sound_result
mixer_sound_reserve(const char *sound_file_path, bool active_on_start, bool loop) {
  log_warnlf("%s: this is not thread-safe yet", __func__);
  if (loop) log_warnlf("%s: not loopable yet", __func__);
  struct sound *sound = arena_array_grow(g_mixer.sounds, 1);
  if (!sound) {
    log_errorlf("%s: couldn't allocate sound", __func__);
    return (struct sound_result) { 0, false };
  }
  ma_decoder_config decoder_config = ma_decoder_config_init(
    g_mixer.device.playback.format,
    g_mixer.device.playback.channels,
    g_mixer.device.sampleRate
  );
  if (ma_decoder_init_file(sound_file_path, &decoder_config, &sound->decoder) != MA_SUCCESS) {
    log_errorlf("%s: couldn't initiate sound decoder", __func__);
    return (struct sound_result) { 0, false };
  }
  if (loop) {
    if (ma_data_source_set_looping(&sound->decoder, true) != MA_SUCCESS) {
      log_warnlf("%s: couldn't set sound to loop", __func__);
    }
  }
  sound->active = active_on_start;
  return (struct sound_result) { arena_array_length(g_mixer.sounds) - 1, true };
}

void
mixer_clear_sounds(void) {
  uint32_t sounds_length = arena_array_length(g_mixer.sounds);
  for (uint32_t i = 0; i < sounds_length; i++) {
    (void)ma_decoder_uninit(&g_mixer.sounds[i].decoder);
  }
  arena_array_clear(g_mixer.sounds);
}
