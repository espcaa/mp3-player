#ifndef THEME_H
#define THEME_H

#include <stdint.h>

typedef struct {
  uint32_t bg;          // screen background
  uint32_t surface;     // bars, panels
  uint32_t surface_alt; // placeholders, insets
  uint32_t text;        // primary text
  uint32_t text_dim;    // secondary text
  uint32_t accent;      // highlights, progress
  uint32_t on_accent;   // text/fg drawn over accent fills
} theme_t;

extern const theme_t THEME_ROSE_PINE; // built-in default / fallback
extern const theme_t *g_theme;        // active theme

void theme_init(void);
void theme_load_settings(const char *path);

#endif
