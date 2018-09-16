#include "mgos_mcp320x_internal.h"

struct mgos_mcp320x *mgos_mcp320x_create(struct mgos_spi *spi, uint8_t cs_idx, uint16_t vref) {

  struct mgos_mcp320x *dev;
  dev = calloc(1, sizeof(struct mgos_mcp320x));
  if (!dev) {
    return NULL;
  }

  memset(dev, 0, sizeof(struct mgos_mcp320x));
  dev->spi    = spi;
  dev->cs_idx = cs_idx;
  dev->vref   = vref;

  return dev;
}

uint16_t mgos_mcp320x_read(struct mgos_mcp320x *dev, uint8_t channel) {
  return 0;
  (void) channel;
  (void) dev;
}

void mgos_mcp320x_destroy(struct mgos_mcp320x **dev) {

  if (!*dev) {
    return;
  }

  free(*dev);
  *dev = NULL;
  return;
}

bool mgos_mcp320x_spi_init(void) {
  return true;
}
