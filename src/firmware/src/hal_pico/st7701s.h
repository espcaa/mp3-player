#ifndef ST7701S_H
#define ST7701S_H

// resets the panel and pushes the ER-TFT028A5-4 register init over 3-wire SPI,
// configured for RGB565 (COLMOD 0x55). pixels then come over the RGB bus.
void st7701s_init(void);

#endif
