#include "hw_config.h"

// PIO SDIO. Driver constraint: CLK = D0 - 2, D1/D2/D3 = D0 + 1/2/3, CMD free.
// Board: CLK=40, CMD=41, D0=42, D1=43, D2=44, D3=45.
static sd_sdio_if_t sdio_if = {
    .CMD_gpio = 41,
    .D0_gpio = 42, // -> CLK=40, D1=43, D2=44, D3=45
    .SDIO_PIO = pio1, // audio uses pio0
    .baud_rate = 125 * 1000 * 1000 / 6, // ~20.8 MHz
};

static sd_card_t sd_card = {.type = SD_IF_SDIO, .sdio_if_p = &sdio_if};

size_t sd_get_num(void) { return 1; }

sd_card_t *sd_get_by_num(size_t num) {
  return num == 0 ? &sd_card : NULL;
}
