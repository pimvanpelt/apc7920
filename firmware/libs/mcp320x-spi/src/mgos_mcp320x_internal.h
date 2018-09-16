#pragma once

#include "mgos.h"
#include "mgos_spi.h"
#include "mgos_mcp320x.h"

struct mgos_mcp320x {
  uint16_t vref;
  uint8_t cs_idx;
  struct mgos_spi *spi;
};
