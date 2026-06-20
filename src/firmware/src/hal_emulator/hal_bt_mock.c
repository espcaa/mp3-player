#include "../app/hal_bluetooth.h"
#include <stdio.h>
#include <string.h>

// fake bluetooth stack for the emulator

static bool g_enabled = true;

static bt_device_t g_paired[MAX_BT_DEVICES];
static int g_paired_count = 0;
static int g_connected = -1;

static bt_device_t g_candidates[MAX_BT_DEVICES]; // unpaired, discoverable
static int g_candidate_count = 0;

static bt_device_t g_scan[MAX_BT_DEVICES]; // results of the current scan
static int g_scan_count = 0;

static void add_paired(const char *mac, const char *name, int rssi) {
  if (g_paired_count >= MAX_BT_DEVICES)
    return;
  bt_device_t *d = &g_paired[g_paired_count++];
  snprintf(d->mac_address, sizeof(d->mac_address), "%s", mac);
  snprintf(d->name, sizeof(d->name), "%s", name);
  d->rssi = rssi;
}

static void add_candidate(const char *mac, const char *name, int rssi) {
  if (g_candidate_count >= MAX_BT_DEVICES)
    return;
  bt_device_t *d = &g_candidates[g_candidate_count++];
  snprintf(d->mac_address, sizeof(d->mac_address), "%s", mac);
  snprintf(d->name, sizeof(d->name), "%s", name);
  d->rssi = rssi;
}

void hal_bt_init(void) {
  add_paired("AA:BB:CC:00:00:01", "Sony WH-1000XM5", -45);
  add_paired("AA:BB:CC:00:00:02", "Car Stereo", -70);
  g_connected = 0;

  add_candidate("AA:BB:CC:00:00:10", "JBL Flip 6", -58);
  add_candidate("AA:BB:CC:00:00:11", "AirPods Pro", -66);
  add_candidate("AA:BB:CC:00:00:12", "Garage Boombox", -82);
  add_candidate("AA:BB:CC:00:00:13", "Kitchen Speaker", -75);
}

void hal_bt_tick(void) {}

void hal_bt_set_enabled(bool on) {
  g_enabled = on;
  if (!on) {
    g_connected = -1;
    g_scan_count = 0;
  }
}
bool hal_bt_is_enabled(void) { return g_enabled; }

int hal_bt_paired_count(void) { return g_paired_count; }

bool hal_bt_get_paired(int index, bt_device_t *out) {
  if (index < 0 || index >= g_paired_count || !out)
    return false;
  *out = g_paired[index];
  return true;
}

int hal_bt_connected_index(void) { return g_connected; }

bool hal_bt_connect(int paired_index) {
  if (!g_enabled || paired_index < 0 || paired_index >= g_paired_count)
    return false;
  g_connected = paired_index; // one at a time
  return true;
}

void hal_bt_disconnect(void) { g_connected = -1; }

void hal_bt_forget(int paired_index) {
  if (paired_index < 0 || paired_index >= g_paired_count)
    return;
  if (g_connected == paired_index)
    g_connected = -1;
  else if (g_connected > paired_index)
    g_connected--;
  for (int j = paired_index; j < g_paired_count - 1; j++)
    g_paired[j] = g_paired[j + 1];
  g_paired_count--;
}

void hal_bt_start_scan(void) {
  if (!g_enabled)
    return;
  g_scan_count = g_candidate_count;
  for (int i = 0; i < g_candidate_count; i++)
    g_scan[i] = g_candidates[i];
}

void hal_bt_stop_scan(void) {}

int hal_bt_scan_count(void) { return g_scan_count; }

bool hal_bt_get_scan_result(int index, bt_device_t *out) {
  if (index < 0 || index >= g_scan_count || !out)
    return false;
  *out = g_scan[index];
  return true;
}

bool hal_bt_pair(int scan_index) {
  if (scan_index < 0 || scan_index >= g_scan_count)
    return false;
  bt_device_t d = g_scan[scan_index];
  add_paired(d.mac_address, d.name, d.rssi);
  g_connected = g_paired_count - 1; // connect the freshly paired device

  // drop it from the candidate pool so it won't reappear in later scans
  for (int i = 0; i < g_candidate_count; i++) {
    if (strcmp(g_candidates[i].mac_address, d.mac_address) == 0) {
      for (int j = i; j < g_candidate_count - 1; j++)
        g_candidates[j] = g_candidates[j + 1];
      g_candidate_count--;
      break;
    }
  }
  return true;
}
