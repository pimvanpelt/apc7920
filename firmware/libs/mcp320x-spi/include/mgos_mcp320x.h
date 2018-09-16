#pragma once

#include "mgos.h"
#include "mgos_spi.h"

struct mgos_mcp320x;

struct mgos_mcp320x *mgos_mcp320x_create(struct mgos_spi *spi, uint8_t cs_idx, uint16_t vref);
uint16_t mgos_mcp320x_read(struct mgos_mcp320x *dev, uint8_t channel);
void mgos_mcp320x_destroy(struct mgos_mcp320x **dev);
