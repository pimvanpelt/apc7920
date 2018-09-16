#include "mgos_pcf8574_internal.h"

struct mgos_pcf8574 *mgos_pcf8574_create(struct mgos_i2c *i2c, uint8_t i2c_addr) {

  struct mgos_pcf8574 *dev;
  dev = calloc(1, sizeof(struct mgos_pcf8574));
  if (!dev) {
    return NULL;
  }

  memset(dev, 0, sizeof(struct mgos_pcf8574));
  dev->i2c      = i2c;
  dev->i2c_addr = i2c_addr;

  return dev;
}

void mgos_pcf8574_destroy(struct mgos_pcf8574 **dev) {

  if (!*dev) {
    return;
  }

  free(*dev);
  *dev = NULL;
  return;
}

bool mgos_pcf8574_i2c_init(void) {
  return true;
}

