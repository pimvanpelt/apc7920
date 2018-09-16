#pragma once

#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_pcf8574.h"

struct mgos_pcf8574 {
  uint8_t          i2c_addr;
  struct mgos_i2c *i2c;

  /* bits of output values */
  uint8_t          _output;

  /* bits of input values */
  uint8_t          _input;

  /* directionality (output or input) */
  uint8_t          _mode;
};
