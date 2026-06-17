#ifndef HAL_BLUETOOTH_H
#define HAL_BLUETOOTH_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_BT_DEVICES 10

typedef enum {
  BT_STATE_OFF,
  BT_STATE_IDLE,
  BT_STATE_SCANNING,
  BT_STATE_PAIRING,
  BT_STATE_CONNECTED
} bt_state_t;

typedef struct {
  char mac_address[18];
  char name[32];
  int rssi;
} bt_device_t;

void hal_bt_init(void);
void hal_bt_tick(void);

void hal_bt_start_scan(void);
void hal_bt_stop_scan(void);
void hal_bt_connect(const char *mac_address);
void hal_bt_disconnect(void);

bt_state_t hal_bt_get_state(void);
int hal_bt_get_device_count(void);
bool hal_bt_get_device(int index, bt_device_t *out_device);

#endif
