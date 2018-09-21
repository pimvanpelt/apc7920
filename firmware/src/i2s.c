#include "mgos.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "soc/syscon_reg.h"
#include "driver/adc.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#define SAMPLE_RATE    (4000)
#define I2S_NUM        (0)

static QueueHandle_t s_i2s_event_queue;

void i2s_scanner(void *pvParams) {
  int i2s_read_len = 1024 * 2;
  uint16_t i2s_read_buff[1024];

  while(1) {
    system_event_t evt;
    if (xQueueReceive(s_i2s_event_queue, &evt, portMAX_DELAY) == pdPASS) {
      if (evt.event_id==2) { // I2S_EVENT_RX_DONE
        size_t bytes_read = 0;
	uint32_t samples[16];

        i2s_read(I2S_NUM, (char*)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
//        LOG(LL_INFO, ("Received %u bytes", bytes_read));
	for (int i=0; i<16; i++)
	  samples[i]=0;

	for (int i=0; i < 1024; i++)
	  samples[i2s_read_buff[i]>>12]++;
	
        LOG(LL_INFO, ("%u %u %u %u", samples[4], samples[5], samples[6], samples[7]));

        ESP_LOG_BUFFER_HEX("buf", i2s_read_buff, 32);
      } else {
        LOG(LL_ERROR, ("Event %d received", evt.event_id));
      }
    }
  }
}

void i2s_init() {
  i2s_config_t     i2s_config = {
    .mode                 = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = 16,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .dma_buf_count        = 2,
    .dma_buf_len          = 1024,
    .use_apll             = true,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1
  };

  i2s_driver_install(I2S_NUM, &i2s_config, 2, &s_i2s_event_queue);
  i2s_set_adc_mode(I2S_NUM, ADC1_CHANNEL_0);


  // This 32 bit register has 4 bytes for the first set of channels to scan.
  // Each byte consists of:
  // [7:4] Channel
  // [3:2] Bit Width; 3=12bit, 2=11bit, 1=10bit, 0=9bit
  // [1:0] Attenuation; 3=11dB, 2=6dB, 1=2.5dB, 0=0dB
  // 0x47 == Chan4, 12bit, 11dB
  // 0x57 == Chan5, 12bit, 11dB
  WRITE_PERI_REG(SYSCON_SARADC_SAR1_PATT_TAB1_REG, 0x4F5F6F7F);
//  WRITE_PERI_REG(SYSCON_SARADC_SAR1_PATT_TAB2_REG, 0x00000000);

  // Scan 2 channels.
  SET_PERI_REG_BITS(SYSCON_SARADC_CTRL_REG, SYSCON_SARADC_SAR1_PATT_LEN, 3, SYSCON_SARADC_SAR1_PATT_LEN_S);

  // The raw ADC data is written to DMA in inverted form. Invert back.
  SET_PERI_REG_MASK(SYSCON_SARADC_CTRL2_REG, SYSCON_SARADC_SAR1_INV);

  // It is necessary to add a delay before switching on the ADC for a reliable start (?)
  // WiFi seems to mess with ADC1 otherwise...
  vTaskDelay(8000/portTICK_RATE_MS);
  i2s_adc_enable(I2S_NUM);

  // Start the receiver thread
  xTaskCreate(i2s_scanner, "i2s_scanner", 1024 * 8, NULL, 10, NULL);
}
