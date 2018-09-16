#pragma once

#include "mgos.h"
#include "mgos_i2c.h"

struct mgos_pcf8574;

struct mgos_pcf8574 *mgos_pcf8574_create(struct mgos_i2c *i2c, uint8_t i2c_addr);
void mgos_pcf8574_destroy(struct mgos_pcf8574 **dev);

