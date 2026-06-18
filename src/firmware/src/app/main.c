#include "../ui/components/statusbar.h"
#include "../ui/screen.h"
#include "../ui/screens/library.h"
#include "gfx.h"
#include "hal.h"
#include "hal_bluetooth.h"
#include "player.h"
#include "theme.h"

int main() {
  hal_display_init();
  hal_audio_init(44100);
  hal_bt_init();
  hal_input_init();
  hal_battery_init();

  theme_init();                         // built-in default theme
  theme_load_settings("/settings.ini"); // optional user theme override
  library_init();                       // scan scan

  screen_manager_t sm;
  sm_init(&sm);
  sm_push(&sm, library_screen_create(&sm));

  hal_input_t input;
  uint32_t last_time = hal_get_time_ms();
  int32_t time_accumulator = 0;
  const int32_t MS_PER_FRAME = 16;

  uint32_t last_battery_poll = 0;
  const uint32_t BATTERY_POLL_MS = 2000;

  while (true) {
    uint32_t current_time = hal_get_time_ms();
    int32_t elapsed = current_time - last_time;
    last_time = current_time;
    if (elapsed > 100)
      elapsed = 100;
    time_accumulator += elapsed;

    hal_bt_tick();

    if (current_time - last_battery_poll >= BATTERY_POLL_MS) {
      last_battery_poll = current_time;
      hal_battery_t battery;
      hal_battery_read(&battery);
      if (battery.present)
        status_bar_set_battery(battery.percent, battery.charging);
    }

    while (time_accumulator >= MS_PER_FRAME) {
      hal_input_read(&input);
      player_tick(MS_PER_FRAME); // advance playback before screens read it
      sm_update(&sm, &input, MS_PER_FRAME);
      time_accumulator -= MS_PER_FRAME;
    }

    gfx_clear_screen(g_theme->bg);
    sm_render(&sm);
    hal_display_present();

    uint32_t loop_end = hal_get_time_ms();
    if ((loop_end - current_time) < MS_PER_FRAME)
      hal_delay_ms(1);
  }

  return 0;
}
