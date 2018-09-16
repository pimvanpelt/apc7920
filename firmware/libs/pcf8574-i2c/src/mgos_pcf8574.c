#include "mgos_pcf8574_internal.h"

static bool _write(struct mgos_pcf8574 *dev) {
  uint8_t value;

  if (!dev) {
    return false;
  }

  // Set bits in register for output pins only
  value = (dev->_input & ~dev->_mode) | dev->_output;
  return mgos_i2c_write(dev->i2c, dev->i2c_addr, &value, 1, true);
}

static bool _read(struct mgos_pcf8574 *dev) {
  int ret;

  if (!dev) {
    return false;
  }

  ret = mgos_i2c_read_reg_b(dev->i2c, dev->i2c_addr, 0x01);
  if (ret < 0) {
    return false;
  }
  dev->_input = ret;
  return true;
}

static bool _validpin(int pin) {
  if (pin < 0 || pin > 7) {
    return false;
  }
  return true;
}

struct mgos_pcf8574 *mgos_pcf8574_create(struct mgos_i2c *i2c, uint8_t i2c_addr) {
  struct mgos_pcf8574 *dev;

  dev = calloc(1, sizeof(struct mgos_pcf8574));
  if (!dev) {
    return NULL;
  }

  memset(dev, 0, sizeof(struct mgos_pcf8574));
  dev->i2c      = i2c;
  dev->i2c_addr = i2c_addr;

  // Read initial state
  _read(dev);

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

bool mgos_pcf8574_set_mode(struct mgos_pcf8574 *dev, int pin, enum mgos_gpio_mode mode) {
  if (!dev) {
    return false;
  }
  if (!_validpin(pin)) {
    return false;
  }
  switch (mode) {
  case MGOS_GPIO_MODE_INPUT:
    dev->_mode &= ~(1 << pin);
    break;

  case MGOS_GPIO_MODE_OUTPUT:
    dev->_mode |= (1 << pin);
    break;

  default:
    return false;
  }
  return true;
}

bool mgos_pcf8574_read(struct mgos_pcf8574 *dev, int pin) {
  if (!dev) {
    return false;
  }
  if (!_validpin(pin)) {
    return false;
  }

  _read(dev);
  return dev->_input & (1 << pin);
}

void mgos_pcf8574_write(struct mgos_pcf8574 *dev, int pin, bool level) {
  if (!dev) {
    return;
  }
  if (!_validpin(pin)) {
    return;
  }

  if (level) {
    dev->_output |= (1 << pin);
  } else{
    dev->_output &= ~(1 << pin);
  }
  _write(dev);
  return;
}

bool mgos_pcf8574_toggle(struct mgos_pcf8574 *dev, int pin) {
  if (!dev) {
    return false;
  }
  if (!_validpin(pin)) {
    return false;
  }

  dev->_output ^= (1 << pin);
  _write(dev);
  return dev->_output & (1 << pin);
}

bool mgos_pcf8574_read_out(struct mgos_pcf8574 *dev, int pin) {
  return mgos_pcf8574_read(dev, pin);
}

bool mgos_pcf8574_i2c_init(void) {
  // TODO(pim): init chip
  return true;
}
