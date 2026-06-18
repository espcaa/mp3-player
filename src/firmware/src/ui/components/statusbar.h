#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "../widget.h"

widget_t *status_bar_create();

void status_bar_set_battery(uint8_t percent, bool charging);

#endif
