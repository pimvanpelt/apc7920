#pragma once

#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_gpio.h"

struct mgos_pcf8574;

struct mgos_pcf8574 *mgos_pcf8574_create(struct mgos_i2c *i2c, uint8_t i2c_addr);
void mgos_pcf8574_destroy(struct mgos_pcf8574 **dev);

/* Copied the style from mgos_gpio.h */
bool mgos_pcf8574_set_mode(struct mgos_pcf8574 *dev, int pin, enum mgos_gpio_mode mode);
bool mgos_pcf8574_read(struct mgos_pcf8574 *dev, int pin);
void mgos_pcf8574_write(struct mgos_pcf8574 *dev, int pin, bool level);
bool mgos_pcf8574_toggle(struct mgos_pcf8574 *dev, int pin);
bool mgos_pcf8574_read_out(struct mgos_pcf8574 *dev, int pin);
