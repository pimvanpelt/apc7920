#include "mgos.h"
#include "rpc.h"
#include "adc.h"
#include "channel.h"

enum mgos_app_init_result mgos_app_init(void) {
  channel_init();
  adc_init();
  rpc_init();
  return MGOS_APP_INIT_SUCCESS;
}
