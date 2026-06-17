#include "../app/hal.h"
#include <SDL2/SDL.h>
#include <SDL_scancode.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static uint16_t *framebuffer = NULL;
static uint32_t *sdl_texture_buffer = NULL;

void hal_display_init(void) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n",
            SDL_GetError());
    exit(1);
  }

  window = SDL_CreateWindow("mp3 player emulator", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN);
  if (!window) {
    fprintf(stderr, "Window could not be created! SDL_Error: %s\n",
            SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n",
            SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
                              SCREEN_HEIGHT);

  if (!texture) {
    fprintf(stderr, "Texture could not be created! SDL_Error: %s\n",
            SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  framebuffer =
      (uint16_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
  if (!framebuffer) {
    fprintf(stderr, "Failed to allocate memory for framebuffer!\n");
    exit(1);
  }

  sdl_texture_buffer =
      (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

  memset(framebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
}

uint16_t *hal_get_framebuffer(void) { return framebuffer; }

void hal_display_present(void) {

  for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) {
    uint16_t px = framebuffer[i]; // RGB565

    uint8_t r5 = (px >> 11) & 0x1F;
    uint8_t g6 = (px >> 5) & 0x3F;
    uint8_t b5 = px & 0x1F;

    uint8_t r8 = (r5 << 3) | (r5 >> 2);
    uint8_t g8 = (g6 << 2) | (g6 >> 4);
    uint8_t b8 = (b5 << 3) | (b5 >> 2);
    uint8_t alpha = 0xFF;

    sdl_texture_buffer[i] = (r8 << 24) | (g8 << 16) | (b8 << 8) | alpha;
  }

  SDL_UpdateTexture(texture, NULL, sdl_texture_buffer,
                    SCREEN_WIDTH * sizeof(uint32_t));

  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void hal_audio_init(uint32_t sample_rate) { (void)sample_rate; }
bool hal_audio_ready(void) { return true; }
void hal_audio_submit(const int16_t *samples, size_t num_samples) {
  (void)samples;
  (void)num_samples;
}

// storage
// root is now ./sdcard/ & can be overridden with MP3_SDCARD env var

static const char *sd_root(void) {
  static char root[HAL_MAX_PATH];
  static bool initialized = false;
  if (!initialized) {
    const char *env = getenv("MP3_SDCARD");
    snprintf(root, sizeof(root), "%s", (env && *env) ? env : "sdcard");
    initialized = true;
  }
  return root;
}

static void resolve_path(char *out, size_t n, const char *path) {
  while (*path == '/')
    path++;
  snprintf(out, n, "%s/%s", sd_root(), path);
}

hal_file_t hal_fopen(const char *path, const char *mode) {
  char real[HAL_MAX_PATH];
  resolve_path(real, sizeof(real), path);
  return (hal_file_t)fopen(real, mode);
}
size_t hal_fread(void *ptr, size_t size, size_t count, hal_file_t file) {
  return fread(ptr, size, count, (FILE *)file);
}
int hal_fclose(hal_file_t file) { return fclose((FILE *)file); }

typedef struct {
  DIR *dir;
  char path[HAL_MAX_PATH];
} hal_dir_impl_t;

hal_dir_t hal_diropen(const char *path) {
  hal_dir_impl_t *d = (hal_dir_impl_t *)malloc(sizeof(hal_dir_impl_t));
  if (!d)
    return NULL;
  resolve_path(d->path, sizeof(d->path), path);
  d->dir = opendir(d->path);
  if (!d->dir) {
    free(d);
    return NULL;
  }
  return (hal_dir_t)d;
}

bool hal_dirread(hal_dir_t dir, hal_dirent_t *out) {
  hal_dir_impl_t *d = (hal_dir_impl_t *)dir;
  struct dirent *e;
  while ((e = readdir(d->dir)) != NULL) {
    if (e->d_name[0] == '.')
      continue;

    snprintf(out->name, sizeof(out->name), "%s", e->d_name);

    char full[HAL_MAX_PATH];
    snprintf(full, sizeof(full), "%s/%s", d->path, e->d_name);
    struct stat st;
    if (stat(full, &st) == 0) {
      out->is_dir = S_ISDIR(st.st_mode);
      out->size = (uint32_t)st.st_size;
    } else {
      out->is_dir = false;
      out->size = 0;
    }
    return true;
  }
  return false;
}

void hal_dirclose(hal_dir_t dir) {
  hal_dir_impl_t *d = (hal_dir_impl_t *)dir;
  if (!d)
    return;
  if (d->dir)
    closedir(d->dir);
  free(d);
}

// time

uint32_t hal_get_time_ms(void) { return SDL_GetTicks(); }
void hal_delay_ms(uint32_t ms) { SDL_Delay(ms); }

// input

static hal_input_t internal_state = {0};

void hal_input_init(void) {
  memset(&internal_state, 0, sizeof(internal_state));
}

void hal_input_read(hal_input_t *out_state) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      exit(0);
    }
    if (event.type == SDL_KEYDOWN) {
      if (event.key.repeat)
        continue;

      switch (event.key.keysym.scancode) {
      case SDL_SCANCODE_UP:
        internal_state.dpad_up_pressed = true;
        break;
      case SDL_SCANCODE_DOWN:
        internal_state.dpad_down_pressed = true;
        break;
      case SDL_SCANCODE_LEFT:
        internal_state.dpad_left_pressed = true;
        break;
      case SDL_SCANCODE_RIGHT:
        internal_state.dpad_right_pressed = true;
        break;
      case SDL_SCANCODE_RETURN:
      case SDL_SCANCODE_SPACE:
        internal_state.dpad_center_pressed = true;
        break;
      case SDL_SCANCODE_MINUS:
        internal_state.volume_down_pressed = true;
        break;
      case SDL_SCANCODE_EQUALS:
        internal_state.volume_up_pressed = true;
        break;
      default:
        break;
      }
    }
  }

  const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);
  internal_state.dpad_up = keyboard_state[SDL_SCANCODE_UP];
  internal_state.dpad_down = keyboard_state[SDL_SCANCODE_DOWN];
  internal_state.dpad_left = keyboard_state[SDL_SCANCODE_LEFT];
  internal_state.dpad_right = keyboard_state[SDL_SCANCODE_RIGHT];
  internal_state.dpad_center =
      keyboard_state[SDL_SCANCODE_RETURN] || keyboard_state[SDL_SCANCODE_SPACE];
  internal_state.volume_down = keyboard_state[SDL_SCANCODE_MINUS];
  internal_state.volume_up = keyboard_state[SDL_SCANCODE_EQUALS];

  *out_state = internal_state;

  internal_state.dpad_up_pressed = false;
  internal_state.dpad_down_pressed = false;
  internal_state.dpad_left_pressed = false;
  internal_state.dpad_right_pressed = false;
  internal_state.dpad_center_pressed = false;
  internal_state.volume_down_pressed = false;
  internal_state.volume_up_pressed = false;
}
