#include "mgos.h"
#include "rpc.h"
#include "adc.h"
#include "channel.h"

bool i2s_init();
bool dac_cosine_init();

enum mgos_app_init_result mgos_app_init(void) {
  channel_init();
  adc_init();
  rpc_init();
  dac_cosine_init();

  i2s_init();
  return MGOS_APP_INIT_SUCCESS;
}
