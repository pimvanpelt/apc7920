#pragma once

#include "mgos.h"

bool channel_get(uint8_t idx);
void channel_set(uint8_t idx, bool value);
void channel_toggle(uint8_t idx);
