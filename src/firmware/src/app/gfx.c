#include "gfx.h"
#include "hal.h"

static int clip_x0 = 0, clip_y0 = 0;
static int clip_x1 = SCREEN_WIDTH, clip_y1 = SCREEN_HEIGHT;

void gfx_set_clip(int x, int y, int width, int height) {
  clip_x0 = x < 0 ? 0 : x;
  clip_y0 = y < 0 ? 0 : y;
  clip_x1 = (x + width) > SCREEN_WIDTH ? SCREEN_WIDTH : (x + width);
  clip_y1 = (y + height) > SCREEN_HEIGHT ? SCREEN_HEIGHT : (y + height);
}

void gfx_clear_clip(void) {
  clip_x0 = 0;
  clip_y0 = 0;
  clip_x1 = SCREEN_WIDTH;
  clip_y1 = SCREEN_HEIGHT;
}

static inline uint16_t rgb666_to_565(uint32_t c) {
  uint32_t r6 = (c >> 12) & 0x3F, g6 = (c >> 6) & 0x3F, b6 = c & 0x3F;
  return (uint16_t)(((r6 >> 1) << 11) | (g6 << 5) | (b6 >> 1));
}

void gfx_clear_screen(uint32_t rgb666_color) {
  uint16_t *fb = hal_get_framebuffer();
  uint16_t c = rgb666_to_565(rgb666_color);
  for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) {
    fb[i] = c;
  }
}

void gfx_draw_pixel(int x, int y, uint32_t rgb666_color) {
  if (x < clip_x0 || x >= clip_x1 || y < clip_y0 || y >= clip_y1) {
    return;
  }

  uint16_t *fb = hal_get_framebuffer();
  fb[y * SCREEN_WIDTH + x] = rgb666_to_565(rgb666_color);
}

void gfx_draw_fill_rect(int x, int y, int width, int height,
                        uint32_t rgb666_color) {
  for (int curr_y = y; curr_y < (y + height); curr_y++) {
    for (int curr_x = x; curr_x < (x + width); curr_x++) {
      gfx_draw_pixel(curr_x, curr_y, rgb666_color);
    }
  }
}

void gfx_draw_horizontal_line(int x, int y, int length, uint32_t rgb666_color) {
  for (int curr_x = x; curr_x < (x + length); curr_x++) {
    gfx_draw_pixel(curr_x, y, rgb666_color);
  }
}

void gfx_draw_rect(int x, int y, int width, int height, uint32_t rgb666_color) {
  for (int curr_x = x; curr_x < (x + width); curr_x++) {
    gfx_draw_pixel(curr_x, y, rgb666_color);
    gfx_draw_pixel(curr_x, y + height - 1, rgb666_color);
  }
  for (int curr_y = y; curr_y < (y + height); curr_y++) {
    gfx_draw_pixel(x, curr_y, rgb666_color);
    gfx_draw_pixel(x + width - 1, curr_y, rgb666_color);
  }
}

// text

void gfx_draw_char(int16_t x, int16_t y, char c, const gfx_font_t *font,
                   uint32_t color, uint8_t scale) {
  if (c < font->start_char || c > font->end_char)
    return;
  Glyph g = font->glyphs[c - font->start_char];
  int bytes_per_row = (g.width + 7) / 8;
  int y_top = y - (g.height + g.yoff) * scale;
  for (int row = 0; row < g.height; row++) {
    for (int col = 0; col < g.width; col++) {
      int byte_index = (row * bytes_per_row) + (col / 8);
      int bit_index = 7 - (col % 8);
      if (g.data[byte_index] & (1 << bit_index)) {
        gfx_draw_fill_rect(x + (col * scale), y_top + (row * scale), scale,
                           scale, color);
      }
    }
  }
}

void gfx_draw_string(int16_t x, int16_t y, const char *str,
                     const gfx_font_t *font, uint32_t color, uint8_t scale) {
  int16_t cursor_x = x;
  while (*str) {
    gfx_draw_char(cursor_x, y, *str, font, color, scale);
    Glyph g = font->glyphs[*str - font->start_char];
    cursor_x += g.advance * scale;
    str++;
  }
}

// bmp

static uint16_t rd16(const uint8_t *p) { return p[0] | (p[1] << 8); }
static uint32_t rd32(const uint8_t *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

bool gfx_draw_bmp(int x, int y, int dst_w, int dst_h, const char *path) {
  hal_file_t f = hal_fopen(path, "rb");
  if (!f)
    return false;

  uint8_t hdr[54];
  if (hal_fread(hdr, 1, 54, f) != 54 || hdr[0] != 'B' || hdr[1] != 'M') {
    hal_fclose(f);
    return false;
  }

  uint32_t data_off = rd32(hdr + 10);
  int src_w = (int32_t)rd32(hdr + 18);
  int src_h = (int32_t)rd32(hdr + 22);
  uint16_t bpp = rd16(hdr + 28);
  uint32_t comp = rd32(hdr + 30);

  bool top_down = src_h < 0;
  if (src_h < 0)
    src_h = -src_h;

  if (comp != 0 || (bpp != 24 && bpp != 32) || src_w <= 0 || src_h <= 0 ||
      src_w > SCREEN_WIDTH) {
    hal_fclose(f);
    return false;
  }

  int bpp_bytes = bpp / 8;
  int row_size = ((bpp * src_w + 31) / 32) * 4; // rows padded to 4 bytes
  int out_w = dst_w > 0 ? dst_w : src_w;
  int out_h = dst_h > 0 ? dst_h : src_h;

  for (uint32_t i = 54; i < data_off; i++) {
    uint8_t junk;
    if (hal_fread(&junk, 1, 1, f) != 1) {
      hal_fclose(f);
      return false;
    }
  }

  static uint8_t row[SCREEN_WIDTH * 4];
  for (int fr = 0; fr < src_h; fr++) {
    if (hal_fread(row, 1, row_size, f) != (size_t)row_size)
      break;

    int img_row = top_down ? fr : (src_h - 1 - fr);
    int o0 = (img_row * out_h + src_h - 1) / src_h;
    int o1 = ((img_row + 1) * out_h + src_h - 1) / src_h - 1;
    for (int o = o0; o <= o1; o++) {
      if (o < 0 || o >= out_h)
        continue;
      for (int c = 0; c < out_w; c++) {
        const uint8_t *px = row + (c * src_w / out_w) * bpp_bytes; // BGR(A)
        uint32_t rgb666 = ((uint32_t)(px[2] >> 2) << 12) |
                          ((uint32_t)(px[1] >> 2) << 6) | (px[0] >> 2);
        gfx_draw_pixel(x + c, y + o, rgb666);
      }
    }
  }

  hal_fclose(f);
  return true;
}
