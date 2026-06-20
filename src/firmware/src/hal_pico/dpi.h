#ifndef DPI_H
#define DPI_H

#include <stdint.h>

// starts the continuous RGB565 DPI scanout of `framebuffer` (PSRAM) on pio2 +
// dma. self-restarting; runs forever once called.
void dpi_start(const uint16_t *framebuffer);

#endif
