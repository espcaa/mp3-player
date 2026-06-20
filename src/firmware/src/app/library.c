#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MUSIC_ROOT "/music"

library_t g_library;

static void set_field(char *dst, size_t n, const char *val) {
  snprintf(dst, n, "%s", val); // safe truncating copy
}

static bool ends_with(const char *s, const char *suffix) {
  size_t ls = strlen(s), lx = strlen(suffix);
  return ls >= lx && strcmp(s + ls - lx, suffix) == 0;
}

static track_t *add_track(void) {
  if (g_library.count >= g_library.capacity) {
    int cap = g_library.capacity ? g_library.capacity * 2 : 32;
    track_t *grown = realloc(g_library.tracks, cap * sizeof(track_t));
    if (!grown)
      return NULL;
    g_library.tracks = grown;
    g_library.capacity = cap;
  }
  track_t *t = &g_library.tracks[g_library.count++];
  memset(t, 0, sizeof(*t));
  return t;
}

static void parse_sidecar(const char *path, track_t *t) {
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

    char *eq = strchr(line, '=');
    if (!eq)
      continue;
    *eq = '\0';
    const char *key = line, *val = eq + 1;

    if (strcmp(key, "title") == 0)
      set_field(t->title, sizeof(t->title), val);
    else if (strcmp(key, "artist") == 0)
      set_field(t->artist, sizeof(t->artist), val);
    else if (strcmp(key, "album") == 0)
      set_field(t->album, sizeof(t->album), val);
    else if (strcmp(key, "track") == 0)
      t->track = atoi(val);
    else if (strcmp(key, "duration") == 0)
      t->duration_ms = (uint32_t)strtoul(val, NULL, 10);
    else if (strcmp(key, "color") == 0)
      set_field(t->color, sizeof(t->color), val);
  }
}

static void scan_album(const char *album_path) {
  hal_dir_t dir = hal_diropen(album_path);
  if (!dir)
    return;
  hal_dirent_t e;
  while (hal_dirread(dir, &e)) {
    if (e.is_dir || !ends_with(e.name, ".txt"))
      continue; // one track per .txt sidecar

    track_t *t = add_track();
    if (!t)
      break;

    char sidecar[HAL_MAX_PATH];
    snprintf(sidecar, sizeof(sidecar), "%s/%s", album_path, e.name);

    // track path = sidecar with ".txt" swapped for ".mp3"
    snprintf(t->path, sizeof(t->path), "%s", sidecar);
    size_t L = strlen(t->path);
    if (L >= 4)
      memcpy(t->path + L - 4, ".mp3", 4);

    // default title = filename without extension; sidecar may override
    set_field(t->title, sizeof(t->title), e.name);
    char *dot = strrchr(t->title, '.');
    if (dot)
      *dot = '\0';

    snprintf(t->cover_small, sizeof(t->cover_small), "%s/cover_80.bmp",
             album_path);
    snprintf(t->cover_large, sizeof(t->cover_large), "%s/cover_300.bmp",
             album_path);

    parse_sidecar(sidecar, t);
  }
  hal_dirclose(dir);
}

static void scan_artist(const char *artist_path) {
  hal_dir_t dir = hal_diropen(artist_path);
  if (!dir)
    return;
  hal_dirent_t e;
  while (hal_dirread(dir, &e)) {
    if (!e.is_dir)
      continue;
    char album_path[HAL_MAX_PATH];
    snprintf(album_path, sizeof(album_path), "%s/%s", artist_path, e.name);
    scan_album(album_path);
  }
  hal_dirclose(dir);
}

void library_init(void) {
  g_library.tracks = NULL;
  g_library.count = 0;
  g_library.capacity = 0;

  hal_dir_t dir = hal_diropen(MUSIC_ROOT);
  if (!dir)
    return;
  hal_dirent_t e;
  while (hal_dirread(dir, &e)) {
    if (!e.is_dir)
      continue;
    char artist_path[HAL_MAX_PATH];
    snprintf(artist_path, sizeof(artist_path), "%s/%s", MUSIC_ROOT, e.name);
    scan_artist(artist_path);
  }
  hal_dirclose(dir);
}

void library_free(void) {
  free(g_library.tracks);
  g_library.tracks = NULL;
  g_library.count = 0;
  g_library.capacity = 0;
}
