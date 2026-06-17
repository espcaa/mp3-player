#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HEX_TO_RGB666(hex)                                                     \
  (((((hex) >> 18) & 0x3F) << 12) | ((((hex) >> 10) & 0x3F) << 6) |            \
   ((((hex) >> 2) & 0x3F)))

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 640

void hal_display_init(void);
// framebuffer stores in 16 bit rb656
// gfx still speaks rgb666, so we convert to 565 on write
uint16_t *hal_get_framebuffer(void);
void hal_display_present(void);

void hal_audio_init(uint32_t sample_rate);
bool hal_audio_ready(void);
void hal_audio_submit(const int16_t *samples, size_t num_samples);

// storage root is /
#define HAL_MAX_PATH 256
#define HAL_MAX_NAME 64

typedef void *hal_file_t;
hal_file_t hal_fopen(const char *path, const char *mode);
size_t hal_fread(void *ptr, size_t size, size_t count, hal_file_t file);
int hal_fclose(hal_file_t file);

typedef struct {
  char name[HAL_MAX_NAME];
  bool is_dir;
  uint32_t size;
} hal_dirent_t;

typedef void *hal_dir_t;
hal_dir_t hal_diropen(const char *path);

bool hal_dirread(hal_dir_t dir, hal_dirent_t *out);
void hal_dirclose(hal_dir_t dir);

uint32_t hal_get_time_ms(void);
void hal_delay_ms(uint32_t ms);

// input

typedef struct {
  bool dpad_up;
  bool dpad_down;
  bool dpad_left;
  bool dpad_right;
  bool dpad_center;
  bool volume_down;
  bool volume_up;

  bool dpad_up_pressed;
  bool dpad_down_pressed;
  bool dpad_left_pressed;
  bool dpad_right_pressed;
  bool dpad_center_pressed;
  bool volume_down_pressed;
  bool volume_up_pressed;
} hal_input_t;

void hal_input_read(hal_input_t *out_state);

void hal_input_init(void);

#endif
