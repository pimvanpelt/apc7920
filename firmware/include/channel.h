#pragma once

#include "mgos.h"

struct channel_t {
  uint8_t idx;
  bool    relay_state;
  double  last_change;
};

bool channel_init(void);
bool channel_get(uint8_t idx);
void channel_set(uint8_t idx, bool value);
void channel_toggle(uint8_t idx);
