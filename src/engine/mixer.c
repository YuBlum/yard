#include <miniaudio.h>
#include "engine/core.h"
#include "engine/arena.h"

struct mixer {
  ma_device device;
  struct arena *tmp_buffer_arena;
  struct arena *inactive_decoders_arena;
  ma_decoder   *inactive_decoders;
  struct arena *active_decoders_arena;
  ma_decoder   *active_decoders;
  struct arena *to_be_inactive_decoders_arena;
  bool         *to_be_inactive_decoders;
};
#define TMP_BUFFER_ARENA_CAPACITY (1ul<<20)

static struct mixer g_mixer;

static void
data_callback(ma_device *device, void *output, const void *input, uint32_t frame_count) {
  (void)device;
  (void)output;
  (void)input;
  (void)frame_count;
  float *tmp = arena_push_array(g_mixer.tmp_buffer_arena, true, float, TMP_BUFFER_ARENA_CAPACITY);
  if (!tmp) {
    log_errorl("couldn't allocate temporary buffer for mixing");
    return;
  }
  /*
  struct sounds *sounds = device->pUserData;
  float *out = output;
  for (uint32_t i = 0; i < sounds->amount; i++) {
    if (sounds->data[i].paused) continue;
    (void)memset(sounds->tmp_buff, 0, frame_count * sizeof (float));
    ma_result res = ma_decoder_read_pcm_frames(&sounds->data[i].decoder, sounds->tmp_buff, frame_count, 0);
    if (res != MA_SUCCESS) continue;
    for (uint32_t sample = 0; sample < frame_count; sample++) {
      out[sample] += sounds->tmp_buff[sample];
      if (out[sample] > 1.0f) out[sample] = 1.0f;
      if (out[sample] < -1.0f) out[sample] = -1.0f;
    }
  }
  */
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
  //config.pUserData         = ;
  (void)config;
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
  log_infol("mixer temporary buffer arena created!");
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
