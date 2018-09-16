#include "channel.h"
#include "mgos_pcf8574.h"

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


// TODO(pim): Read channel state from PCF8574
bool channel_init() {
  memset(s_channel, -1, sizeof(s_channel));
  return true;
}
