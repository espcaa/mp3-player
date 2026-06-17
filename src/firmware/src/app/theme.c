#include "theme.h"
#include "hal.h" // HEX_TO_RGB666, HAL_MAX_PATH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const theme_t THEME_ROSE_PINE = {
    .bg = HEX_TO_RGB666(0x191724),
    .surface = HEX_TO_RGB666(0x1F1D2E),
    .surface_alt = HEX_TO_RGB666(0x26233A),
    .text = HEX_TO_RGB666(0xE0DEF4),
    .text_dim = HEX_TO_RGB666(0x908CAA),
    .accent = HEX_TO_RGB666(0xEB6F92),
    .on_accent = HEX_TO_RGB666(0x191724),
};

static theme_t g_active;
const theme_t *g_theme = &g_active;

void theme_init(void) { g_active = THEME_ROSE_PINE; }

// reads settings.ini TODO: move to a dedicated settings thing
static void parse_ini(const char *path,
                      void (*on_kv)(const char *key, const char *val,
                                    void *user),
                      void *user) {
  hal_file_t f = hal_fopen(path, "r");
  if (!f)
    return;
  char buf[1024];
  size_t n = hal_fread(buf, 1, sizeof(buf) - 1, f);
  hal_fclose(f);
  buf[n] = '\0';

  char *p = buf;
  while (*p) {
    char *line = p;
    char *nl = strchr(p, '\n');
    if (nl) {
      *nl = '\0';
      p = nl + 1;
    } else {
      p += strlen(p);
    }
    char *cr = strchr(line, '\r');
    if (cr)
      *cr = '\0';
    if (line[0] == '#' || line[0] == ';' || line[0] == '\0')
      continue; // comment / blank

    char *eq = strchr(line, '=');
    if (!eq)
      continue;
    *eq = '\0';
    on_kv(line, eq + 1, user);
  }
}

static void color_kv(const char *key, const char *val, void *user) {
  theme_t *t = (theme_t *)user;
  uint32_t c = HEX_TO_RGB666((uint32_t)strtol(val, NULL, 16));
  if (strcmp(key, "bg") == 0)
    t->bg = c;
  else if (strcmp(key, "surface") == 0)
    t->surface = c;
  else if (strcmp(key, "surface_alt") == 0)
    t->surface_alt = c;
  else if (strcmp(key, "text") == 0)
    t->text = c;
  else if (strcmp(key, "text_dim") == 0)
    t->text_dim = c;
  else if (strcmp(key, "accent") == 0)
    t->accent = c;
  else if (strcmp(key, "on_accent") == 0)
    t->on_accent = c;
}

static void settings_kv(const char *key, const char *val, void *user) {
  char *name = (char *)user;
  if (strcmp(key, "theme") == 0)
    snprintf(name, HAL_MAX_NAME, "%s", val);
}

void theme_load_settings(const char *path) {
  char name[HAL_MAX_NAME] = {0};
  parse_ini(path, settings_kv, name);
  if (!name[0])
    return; // no theme named -> keep the default

  char theme_path[HAL_MAX_PATH];
  snprintf(theme_path, sizeof(theme_path), "/themes/%s.ini", name);
  parse_ini(theme_path, color_kv, &g_active); // overlay onto active
}
