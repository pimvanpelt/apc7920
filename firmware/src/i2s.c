#include "mgos.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"

#define SAMPLE_RATE    (48000)
#define I2S_NUM        (0)

void i2s_task(void *pvParams) {
  i2s_config_t     i2s_config = {
    .mode                 = I2S_MODE_MASTER | I2S_MODE_RX,                      // Only TX
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = 16,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,                         //2-channels
    .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
    .dma_buf_count        = 6,
    .dma_buf_len          = 60,
    .use_apll             = false,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1                                //Interrupt level 1
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num   = 26,
    .ws_io_num    = 25,
    .data_out_num = 22,
    .data_in_num  = -1                                                          //Not used
  };

  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
}

void i2s_init() {
  return;
}
