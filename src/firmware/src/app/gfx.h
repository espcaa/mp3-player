#ifndef GFX_H
#define GFX_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t width;
  uint8_t height;
  uint8_t advance;
  int8_t yoff;
  const uint8_t *data;
} Glyph;

typedef struct {
  const Glyph *glyphs;
  uint8_t start_char;
  uint8_t end_char;
  uint8_t height;
} gfx_font_t;

void gfx_clear_screen(uint32_t rgb666_color);

void gfx_set_clip(int x, int y, int width, int height);
void gfx_clear_clip(void);

void gfx_draw_pixel(int x, int y, uint32_t rgb666_color);
void gfx_draw_fill_rect(int x, int y, int width, int height,
                        uint32_t rgb666_color);
void gfx_draw_rect(int x, int y, int width, int height, uint32_t rgb666_color);
void gfx_draw_horizontal_line(int x, int y, int length, uint32_t rgb666_color);
void gfx_draw_string(int16_t x, int16_t y, const char *str,
                     const gfx_font_t *font, uint32_t color, uint8_t scale);
void gfx_draw_char(int16_t x, int16_t y, char c, const gfx_font_t *font,
                   uint32_t color, uint8_t scale);

bool gfx_draw_bmp(int x, int y, int dst_w, int dst_h, const char *path);

#endif
