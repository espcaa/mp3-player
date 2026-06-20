#ifndef HAL_BLUETOOTH_H
#define HAL_BLUETOOTH_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_BT_DEVICES 10

typedef struct {
  char mac_address[18];
  char name[32];
  int rssi;
} bt_device_t;

void hal_bt_init(void);
void hal_bt_tick(void);

void hal_bt_set_enabled(bool on);
bool hal_bt_is_enabled(void);

int hal_bt_paired_count(void);
bool hal_bt_get_paired(int index, bt_device_t *out);
int hal_bt_connected_index(void);      // -1 if none
bool hal_bt_connect(int paired_index); // disconnects any existing connection
void hal_bt_disconnect(void);
void hal_bt_forget(int paired_index);

// scanning for new devices
void hal_bt_start_scan(void);
void hal_bt_stop_scan(void);
int hal_bt_scan_count(void);
bool hal_bt_get_scan_result(int index, bt_device_t *out);
bool hal_bt_pair(int scan_index); // pair + connect a discovered device

#endif
