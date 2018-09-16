#pragma once

#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_pcf8574.h"

struct mgos_pcf8574 {
  uint8_t i2c_addr;
  struct mgos_i2c *i2c;
};
