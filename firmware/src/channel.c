#include "channel.h"

static struct channel_t s_channel[8];

bool channel_get(uint8_t idx) {
  return false;
  (void) idx;
}

void channel_set(uint8_t idx, bool value) {
  return;
  (void) idx;
  (void) value;
}

void channel_toggle(uint8_t idx) {
  return;
  (void) idx;
}

bool channel_init() {
  memset(s_channel, -1, sizeof(s_channel));
  return true;
}
