#include "dpi.h"
#include "../app/hal.h"
#include "dpi.pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

#define DPI_PIO pio2 // pio0 = audio, pio1 = sdio
#define DATA_BASE 20 // DB0..DB15 on GPIO20..35 (B/G/R contiguous)
#define DATA_COUNT 16
#define CTRL_BASE 16 // PCLK=16, DE=17, VS=18, HS=19 (side-set)
#define CTRL_COUNT 4
#define TARGET_PCLK_HZ 16000000u // TODO: tune on hardware

static const uint16_t *s_fb_addr; // self-restart DMA reads the addr from here

void dpi_start(const uint16_t *fb) {
  s_fb_addr = fb;

  uint offset = pio_add_program(DPI_PIO, &dpi_program);
  uint sm = pio_claim_unused_sm(DPI_PIO, true);

  for (uint p = CTRL_BASE; p < CTRL_BASE + CTRL_COUNT; p++)
    pio_gpio_init(DPI_PIO, p);
  for (uint p = DATA_BASE; p < DATA_BASE + DATA_COUNT; p++)
    pio_gpio_init(DPI_PIO, p);
  pio_sm_set_consecutive_pindirs(DPI_PIO, sm, CTRL_BASE, CTRL_COUNT, true);
  pio_sm_set_consecutive_pindirs(DPI_PIO, sm, DATA_BASE, DATA_COUNT, true);

  pio_sm_config c = dpi_program_get_default_config(offset);
  sm_config_set_out_pins(&c, DATA_BASE, DATA_COUNT);
  sm_config_set_sideset_pins(&c, CTRL_BASE);
  sm_config_set_out_shift(&c, true, true, 32); // shift right, autopull @ 32
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  float div = (float)clock_get_hz(clk_sys) / (TARGET_PCLK_HZ * 2.0f);
  sm_config_set_clkdiv(&c, div);
  pio_sm_init(DPI_PIO, sm, offset, &c);
  pio_sm_set_enabled(DPI_PIO, sm, true);

  // self-restarting DMA: A streams the frame; B re-arms A's read address
  int a = dma_claim_unused_channel(true);
  int b = dma_claim_unused_channel(true);

  dma_channel_config ca = dma_channel_get_default_config(a);
  channel_config_set_transfer_data_size(&ca, DMA_SIZE_32); // 2 pixels / word
  channel_config_set_read_increment(&ca, true);
  channel_config_set_write_increment(&ca, false);
  channel_config_set_dreq(&ca, pio_get_dreq(DPI_PIO, sm, true));
  channel_config_set_chain_to(&ca, b);
  dma_channel_configure(a, &ca, &DPI_PIO->txf[sm], fb,
                        (uint)SCREEN_WIDTH * SCREEN_HEIGHT / 2, false);

  dma_channel_config cb = dma_channel_get_default_config(b);
  channel_config_set_transfer_data_size(&cb, DMA_SIZE_32);
  channel_config_set_read_increment(&cb, false);
  channel_config_set_write_increment(&cb, false);
  channel_config_set_chain_to(&cb, b); // self = no chain
  dma_channel_configure(b, &cb, &dma_hw->ch[a].al3_read_addr_trig, &s_fb_addr, 1,
                        false);

  dma_channel_start(a);
}
