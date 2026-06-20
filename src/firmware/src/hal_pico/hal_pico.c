#include "../app/hal.h"
#include "dpi.h"
#include "ff.h"
#include "hardware/i2c.h"
#include "pico/audio_i2s.h"
#include "pico/stdlib.h"
#include "sfe_psram.h"
#include "st7701s.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PSRAM_BASE 0x11000000u
#define PSRAM_CS_PIN 0
#define FB_BYTES ((size_t)SCREEN_WIDTH * SCREEN_HEIGHT * 2)
#define BACKLIGHT_PIN 12

// time
uint32_t hal_get_time_ms(void) { return to_ms_since_boot(get_absolute_time()); }
void hal_delay_ms(uint32_t ms) { sleep_ms(ms); }

// display
// TODO: 18-bit RGB panel via PIO+DMA
static uint16_t *g_fb = NULL;

void hal_display_init(void) {
  stdio_init_all();
  size_t psram = sfe_setup_psram(PSRAM_CS_PIN);
  if (psram >= FB_BYTES)
    g_fb = (uint16_t *)PSRAM_BASE;

  st7701s_init();

  gpio_init(BACKLIGHT_PIN); // TODO: PWM for brightness
  gpio_set_dir(BACKLIGHT_PIN, GPIO_OUT);
  gpio_put(BACKLIGHT_PIN, 1);

  if (g_fb)
    dpi_start(g_fb); // continuous RGB565 scanout (pio2 + dma)
}
uint16_t *hal_get_framebuffer(void) { return g_fb; }
void hal_display_present(void) {}

// audio: I2S to a PCM5102A via pico_audio_i2s (pio0 + dma0). The DAC runs off
// its internal PLL, so MCLK isn't driven (SCK held low).
#define I2S_DATA_PIN 6
#define I2S_CLK_BASE 7 // BCLK=7, LRCLK=8
#define I2S_SCK_PIN 9
#define AUDIO_CHANNELS 2
#define AUDIO_BUF_SAMPLES 1152 // one mp3 frame per channel
#define AUDIO_BUF_COUNT 4

static audio_buffer_pool_t *s_audio_pool = NULL;
static audio_buffer_t *s_audio_cur = NULL;

void hal_audio_init(uint32_t sample_rate) {
  gpio_init(I2S_SCK_PIN);
  gpio_set_dir(I2S_SCK_PIN, GPIO_OUT);
  gpio_put(I2S_SCK_PIN, 0);

  static audio_format_t fmt;
  fmt.sample_freq = sample_rate;
  fmt.format = AUDIO_BUFFER_FORMAT_PCM_S16;
  fmt.channel_count = AUDIO_CHANNELS;

  static audio_buffer_format_t buf_fmt;
  buf_fmt.format = &fmt;
  buf_fmt.sample_stride = AUDIO_CHANNELS * sizeof(int16_t);

  s_audio_pool =
      audio_new_producer_pool(&buf_fmt, AUDIO_BUF_COUNT, AUDIO_BUF_SAMPLES);

  audio_i2s_config_t cfg = {
      .data_pin = I2S_DATA_PIN,
      .clock_pin_base = I2S_CLK_BASE,
      .dma_channel = 0,
      .pio_sm = 0,
  };
  audio_i2s_setup(&fmt, &cfg);
  audio_i2s_connect(s_audio_pool);
  audio_i2s_set_enabled(true);
}

bool hal_audio_ready(void) {
  if (!s_audio_pool)
    return false;
  if (!s_audio_cur)
    s_audio_cur = take_audio_buffer(s_audio_pool, false);
  return s_audio_cur != NULL;
}

void hal_audio_submit(const int16_t *samples, size_t num_samples) {
  if (!s_audio_pool)
    return;
  if (!s_audio_cur) {
    s_audio_cur = take_audio_buffer(s_audio_pool, false);
    if (!s_audio_cur)
      return;
  }
  uint32_t frames = (uint32_t)num_samples / AUDIO_CHANNELS;
  if (frames > s_audio_cur->max_sample_count)
    frames = s_audio_cur->max_sample_count;
  memcpy(s_audio_cur->buffer->bytes, samples,
         frames * AUDIO_CHANNELS * sizeof(int16_t));
  s_audio_cur->sample_count = frames;
  give_audio_buffer(s_audio_pool, s_audio_cur);
  s_audio_cur = NULL;
}

// storage: SD card over PIO SDIO + FatFs (carlk3 library), mounted lazily
static FATFS s_fs;
static bool s_sd_mounted = false;

static bool sd_ensure_mounted(void) {
  if (s_sd_mounted)
    return true;
  FRESULT fr = f_mount(&s_fs, "", 1);
  if (fr != FR_OK) {
    printf("sd: f_mount failed (%d)\n", fr);
    return false;
  }
  s_sd_mounted = true;
  return true;
}

hal_file_t hal_fopen(const char *path, const char *mode) {
  if (!sd_ensure_mounted())
    return NULL;
  BYTE flags = FA_READ;
  if (mode && strchr(mode, 'w'))
    flags = FA_WRITE | FA_CREATE_ALWAYS;
  else if (mode && strchr(mode, 'a'))
    flags = FA_WRITE | FA_OPEN_APPEND;
  FIL *fp = calloc(1, sizeof(FIL));
  if (!fp)
    return NULL;
  if (f_open(fp, path, flags) != FR_OK) {
    free(fp);
    return NULL;
  }
  return (hal_file_t)fp;
}

size_t hal_fread(void *ptr, size_t size, size_t count, hal_file_t file) {
  if (!file || size == 0)
    return 0;
  UINT br = 0;
  if (f_read((FIL *)file, ptr, (UINT)(size * count), &br) != FR_OK)
    return 0;
  return br / size;
}

int hal_fclose(hal_file_t file) {
  if (!file)
    return -1;
  FRESULT fr = f_close((FIL *)file);
  free(file);
  return fr == FR_OK ? 0 : -1;
}

hal_dir_t hal_diropen(const char *path) {
  if (!sd_ensure_mounted())
    return NULL;
  DIR *dp = calloc(1, sizeof(DIR));
  if (!dp)
    return NULL;
  if (f_opendir(dp, path) != FR_OK) {
    free(dp);
    return NULL;
  }
  return (hal_dir_t)dp;
}

bool hal_dirread(hal_dir_t dir, hal_dirent_t *out) {
  if (!dir || !out)
    return false;
  FILINFO fno;
  if (f_readdir((DIR *)dir, &fno) != FR_OK || fno.fname[0] == 0)
    return false;
  snprintf(out->name, sizeof(out->name), "%s", fno.fname);
  out->is_dir = (fno.fattrib & AM_DIR) != 0;
  out->size = (uint32_t)fno.fsize;
  return true;
}

void hal_dirclose(hal_dir_t dir) {
  if (!dir)
    return;
  f_closedir((DIR *)dir);
  free(dir);
}

// --- shared i2c1 bus: TCA9555 button expander + MAX17048 fuel gauge ---
#define I2C_PORT i2c1
#define I2C_SDA 38
#define I2C_SCL 39
#define I2C_BAUD 400000

#define TCA9555_ADDR 0x20 // A2/A1/A0 -> GND
#define BTN_ACTIVE_LOW 1  // pressed pulls the line low

#define BTN_POWER 0   // P00
#define BTN_VOL_UP 9  // P11
#define BTN_VOL_DN 10 // P12
#define BTN_UP 11     // P13
#define BTN_DOWN 12   // P14
#define BTN_LEFT 13   // P15
#define BTN_RIGHT 14  // P16
#define BTN_CENTER 15 // P17

static bool s_i2c_ready = false;

static void i2c_scan(void) {
  for (uint8_t a = 0x08; a < 0x78; a++) {
    uint8_t d;
    if (i2c_read_blocking(I2C_PORT, a, &d, 1, false) >= 0)
      printf("i2c: found device 0x%02X\n", a);
  }
}

static void i2c_bus_init(void) {
  if (s_i2c_ready)
    return;
  i2c_init(I2C_PORT, I2C_BAUD);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
  s_i2c_ready = true;
  i2c_scan(); // TODO: remove if it works
}

void hal_input_init(void) { i2c_bus_init(); }

// raw 16-bit expander (port0 = low byte, port1 = high byte); 0xFFFF on error
static uint16_t read_expander(void) {
  uint8_t reg = 0x00, buf[2];
  if (i2c_write_blocking(I2C_PORT, TCA9555_ADDR, &reg, 1, true) < 0)
    return 0xFFFF;
  if (i2c_read_blocking(I2C_PORT, TCA9555_ADDR, buf, 2, false) < 0)
    return 0xFFFF;
  return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

void hal_input_read(hal_input_t *out) {
  if (!out)
    return;
  static uint16_t prev = 0;
  uint16_t raw = read_expander();
  uint16_t pressed = BTN_ACTIVE_LOW ? (uint16_t)~raw : raw; // 1 = pressed
  uint16_t edge = pressed & (uint16_t)~prev;
  prev = pressed;

  *out = (hal_input_t){0};
  out->dpad_up = (pressed >> BTN_UP) & 1;
  out->dpad_down = (pressed >> BTN_DOWN) & 1;
  out->dpad_left = (pressed >> BTN_LEFT) & 1;
  out->dpad_right = (pressed >> BTN_RIGHT) & 1;
  out->dpad_center = (pressed >> BTN_CENTER) & 1;
  out->volume_down = (pressed >> BTN_VOL_DN) & 1;
  out->volume_up = (pressed >> BTN_VOL_UP) & 1;
  out->power = (pressed >> BTN_POWER) & 1;

  out->dpad_up_pressed = (edge >> BTN_UP) & 1;
  out->dpad_down_pressed = (edge >> BTN_DOWN) & 1;
  out->dpad_left_pressed = (edge >> BTN_LEFT) & 1;
  out->dpad_right_pressed = (edge >> BTN_RIGHT) & 1;
  out->dpad_center_pressed = (edge >> BTN_CENTER) & 1;
  out->volume_down_pressed = (edge >> BTN_VOL_DN) & 1;
  out->volume_up_pressed = (edge >> BTN_VOL_UP) & 1;
  out->power_pressed = (edge >> BTN_POWER) & 1;
}

// battery: MAX17048 fuel gauge for %, expander P06 for charge status
#define MAX17048_ADDR 0x36
#define MAX17048_SOC 0x04
#define CHARGE_BIT 6
#define CHARGE_ACTIVE_LOW 1 // TODO: confirm if active low or high

void hal_battery_init(void) { i2c_bus_init(); }

void hal_battery_read(hal_battery_t *out) {
  if (!out)
    return;
  out->percent = 0;
  out->charging = false;
  out->present = false;

  uint8_t reg = MAX17048_SOC, buf[2];
  if (i2c_write_blocking(I2C_PORT, MAX17048_ADDR, &reg, 1, true) >= 0 &&
      i2c_read_blocking(I2C_PORT, MAX17048_ADDR, buf, 2, false) >= 0) {
    uint16_t pct = (((uint16_t)buf[0] << 8) | buf[1]) / 256; // SOC is 1%/256
    out->percent = pct > 100 ? 100 : (uint8_t)pct;
    out->present = true;
  }

  bool level = (read_expander() >> CHARGE_BIT) & 1;
  out->charging = CHARGE_ACTIVE_LOW ? !level : level;
}
