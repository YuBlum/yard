#ifndef __MIXER_H__
#define __MIXER_H__

bool mixer_make(void);
void mixer_destroy(void);

struct sound_result {
  uint32_t sound_handle;
  bool ok;
};

struct sound_result mixer_sound_reserve(const char *sound_file_path, bool active_on_start, bool loop);
void mixer_clear_sounds(void);

#endif/*__MIXER_H__*/
