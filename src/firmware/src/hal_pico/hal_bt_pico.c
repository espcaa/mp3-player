#include "../app/hal_bluetooth.h"

// TODO: implement the UART interface of the FSC-BT1114QI

void hal_bt_init(void) {}
void hal_bt_tick(void) {}

void hal_bt_set_enabled(bool on) { (void)on; }
bool hal_bt_is_enabled(void) { return false; }

int hal_bt_paired_count(void) { return 0; }
bool hal_bt_get_paired(int index, bt_device_t *out) {
  (void)index;
  (void)out;
  return false;
}
int hal_bt_connected_index(void) { return -1; }
bool hal_bt_connect(int paired_index) {
  (void)paired_index;
  return false;
}
void hal_bt_disconnect(void) {}
void hal_bt_forget(int paired_index) { (void)paired_index; }

void hal_bt_start_scan(void) {}
void hal_bt_stop_scan(void) {}
int hal_bt_scan_count(void) { return 0; }
bool hal_bt_get_scan_result(int index, bt_device_t *out) {
  (void)index;
  (void)out;
  return false;
}
bool hal_bt_pair(int scan_index) {
  (void)scan_index;
  return false;
}
