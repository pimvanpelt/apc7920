#include "mgos.h"
#include "ap7920.h"

enum mgos_app_init_result mgos_app_init(void)
{
  rpc_init();
  return MGOS_APP_INIT_SUCCESS;
}
