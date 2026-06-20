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
// total advance width of str in pixels at the given scale
int gfx_text_width(const char *str, const gfx_font_t *font, uint8_t scale);
// draws str cropped to max_width with a trailing "..." when it overflows;
// max_width <= 0 draws the full string
void gfx_draw_string_ellipsized(int16_t x, int16_t y, const char *str,
                                const gfx_font_t *font, uint32_t color,
                                uint8_t scale, int16_t max_width);
void gfx_draw_char(int16_t x, int16_t y, char c, const gfx_font_t *font,
                   uint32_t color, uint8_t scale);

bool gfx_draw_bmp(int x, int y, int dst_w, int dst_h, const char *path);

// draws each pixel of the bmp with alpha blending against bg_color, using
// alpha_level (0-255) as the opacity
bool gfx_draw_alpha_bmp(int x, int y, int dst_w, int dst_h, const char *path,
                        uint32_t bg_color, uint32_t alpha_level);

typedef struct {
  uint8_t width;
  uint8_t height;
  const uint8_t *data;
} gfx_icon_t;

void gfx_draw_icon(int16_t x, int16_t y, const gfx_icon_t *icon,
                   uint32_t color, uint8_t scale);

#endif
