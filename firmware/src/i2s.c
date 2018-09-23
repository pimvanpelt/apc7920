#include "mgos.h"
#include "mgos_config.h"
#include "mgos_prometheus_metrics.h"
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

#define SAMPLE_RATE     (9000)
#define I2S_NUM         (0)
#define RINGBUF_SIZE    (16)

struct i2c_scanner_adc_channel {
  uint32_t sum;
  uint16_t count;
  uint16_t min;
  uint16_t max;
};

struct i2c_scanner_adc_channel_set {
  struct i2c_scanner_adc_channel chan[8];
};

struct i2c_scanner_adc_channel_ringbuf {
  struct i2c_scanner_adc_channel_set channel_set[RINGBUF_SIZE];
  uint8_t                            head;
};

struct i2c_scanner_stats {
  uint32_t read_usecs;
  uint32_t read_bytes;
  uint32_t read_count;
};

static QueueHandle_t s_i2s_event_queue;

static struct i2c_scanner_stats s_stats;
static struct i2c_scanner_adc_channel_ringbuf s_ringbuf;
static struct mg_connection *s_udp_nc = NULL;

static void prometheus_metrics_fn(struct mg_connection *nc, void *user_data) {
  struct i2c_scanner_adc_channel_set *channel_set = &s_ringbuf.channel_set[s_ringbuf.head];

  mgos_prometheus_metrics_printf(nc, COUNTER, "i2s_scanner_read_count", "Total samples from I2S",
                                 "%u", s_stats.read_count);
  mgos_prometheus_metrics_printf(nc, COUNTER, "i2s_scanner_read_bytes_total", "Total bytes from I2S",
                                 "%u", s_stats.read_bytes);
  mgos_prometheus_metrics_printf(nc, COUNTER, "i2s_scanner_read_usecs_total", "Total time spent in I2S in microseconds",
                                 "%u", s_stats.read_usecs);

  for (int i = 0; i < 8; i++) {
    mgos_prometheus_metrics_printf(nc, GAUGE, "i2s_scanner_raw", "Raw data from I2S Scanner",
                                   "{channel=\"%u\", type=\"min\"} %u", i,
                                   channel_set->chan[i].min);
    mgos_prometheus_metrics_printf(nc, GAUGE, "i2s_scanner_raw", "Raw data from I2S Scanner",
                                   "{channel=\"%u\", type=\"max\"} %u", i,
                                   channel_set->chan[i].max);
    mgos_prometheus_metrics_printf(nc, GAUGE, "i2s_scanner_raw", "Raw data from I2S Scanner",
                                   "{channel=\"%u\", type=\"avg\"} %f", i,
                                   ((float)channel_set->chan[i].sum) / channel_set->chan[i].count);
  }
  (void)user_data;
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data, void *user_data) {
  switch (ev) {
  case MG_EV_RECV: {
//      int bytes_recv = *((int *)ev_data);
//      LOG(LL_INFO, ("Received %d bytes of data on UDP socket", bytes_recv));
    nc->recv_mbuf.len = 0;
    break;
  }

  case MG_EV_SEND: {
//      int bytes_sent = *((int *)ev_data);
//      LOG(LL_INFO, ("Sent %d bytes of data on UDP socket", bytes_sent));
    break;
  }

  default:
    break;
  }
  (void)user_data;
}

static void i2c_scanner_stats(void *args) {
  struct i2c_scanner_adc_channel_set *channel_set = &s_ringbuf.channel_set[s_ringbuf.head];

  for (int i = 0; i < 8; i++) {
    LOG(LL_INFO, ("chan: %d min: %4u max: %4u avg=%.f count=%u",
                  i, channel_set->chan[i].min,
                  channel_set->chan[i].max,
                  ((float)channel_set->chan[i].sum) / channel_set->chan[i].count,
                  channel_set->chan[i].count));
  }
  LOG(LL_INFO, ("stats: bytes=%u count=%u usecs=%u",
                s_stats.read_bytes,
                s_stats.read_count,
                s_stats.read_usecs));

  (void)args;
}

/* This scanner runs at idle priority, grabbing CPU while available.
 * It takes approximately 113ms to complete, and runs at approx 9Hz
 * allowing us to sample the ADC about 90KHz
 */
static void i2s_scanner(void *pvParams) {
  int      i2s_read_len = 1024 * 2;
  uint16_t i2s_read_buff[1024];

  while (1) {
    system_event_t evt;
    if (xQueueReceive(s_i2s_event_queue, &evt, portMAX_DELAY) == pdPASS) {
      if (evt.event_id == 2) { // I2S_EVENT_RX_DONE
        size_t bytes_read = 0;
        struct i2c_scanner_adc_channel_set *channel_set = &s_ringbuf.channel_set[s_ringbuf.head];
        double start = mg_time();

        i2s_read(I2S_NUM, (char *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        taskYIELD();
//        LOG(LL_INFO, ("Received %u bytes", bytes_read));
//        LOG(LL_INFO, ("Reading into channel_set %d", s_ringbuf.head));
        if (s_stats.read_bytes > UINT_MAX - bytes_read) {
          LOG(LL_WARN, ("s_stats.read_bytes overflow"));
          s_stats.read_bytes = 0;
        } else {
          s_stats.read_bytes += bytes_read;
        }

        if (s_stats.read_count == UINT_MAX) {
          LOG(LL_WARN, ("s_stats.read_count overflow"));
          s_stats.read_count = 0;
        } else {
          s_stats.read_count++;
        }

        memset(channel_set->chan, 0, sizeof(struct i2c_scanner_adc_channel) * 8);
        for (int i = 0; i < 8; i++) {
          channel_set->chan[i].min = 0xfff;
        }
        for (int i = 0; i < bytes_read / 2; i++) {
          uint8_t  chan      = (i2s_read_buff[i] >> 12) & 0x07;
          uint16_t adc_value = i2s_read_buff[i] & 0xfff;
          channel_set->chan[chan].count++;
          channel_set->chan[chan].sum += adc_value;
          if (adc_value < channel_set->chan[chan].min) {
            channel_set->chan[chan].min = adc_value;
          }
          if (adc_value > channel_set->chan[chan].max) {
            channel_set->chan[chan].max = adc_value;
          }
        }
        s_ringbuf.head++;
        s_ringbuf.head %= RINGBUF_SIZE;

        if (s_udp_nc) {
          mg_send(s_udp_nc, (void *)i2s_read_buff, bytes_read);
        }

        uint32_t usecs_spent = 1000000 * (mg_time() - start);
        if (s_stats.read_usecs > UINT_MAX - usecs_spent) {
          LOG(LL_WARN, ("s_stats.read_usecs overflow"));
          s_stats.read_usecs = 0;
        } else {
          s_stats.read_usecs += usecs_spent;
        }
      } else {
        LOG(LL_ERROR, ("Event %d received", evt.event_id));
      }
      taskYIELD();
    }
  }
  vTaskDelete(NULL);
}

void i2s_init() {
  i2s_config_t i2s_config = {
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
  WRITE_PERI_REG(SYSCON_SARADC_SAR1_PATT_TAB1_REG, 0x0F1F2F3F);
  WRITE_PERI_REG(SYSCON_SARADC_SAR1_PATT_TAB2_REG, 0x4F5F6F7F);

  // Scan multiple channels.
  SET_PERI_REG_BITS(SYSCON_SARADC_CTRL_REG, SYSCON_SARADC_SAR1_PATT_LEN, 7, SYSCON_SARADC_SAR1_PATT_LEN_S);

  // The raw ADC data is written to DMA in inverted form. Invert back.
  SET_PERI_REG_MASK(SYSCON_SARADC_CTRL2_REG, SYSCON_SARADC_SAR1_INV);

  // It is necessary to add a delay before switching on the ADC for a reliable start (?)
  // WiFi seems to mess with ADC1 otherwise...
  vTaskDelay(8000 / portTICK_RATE_MS);
  i2s_adc_enable(I2S_NUM);

  memset(&s_stats, 0, sizeof(s_stats));
  memset(&s_ringbuf, 0, sizeof(s_ringbuf));

  if (mgos_sys_config_get_app_i2s_sample_enable()) {
    s_udp_nc = mg_connect(mgos_get_mgr(), mgos_sys_config_get_app_i2s_sample_mirror(), ev_handler, NULL);
    if (!s_udp_nc) {
      LOG(LL_ERROR, ("Could not connect to %s", mgos_sys_config_get_app_i2s_sample_mirror()));
    } else {
      LOG(LL_INFO, ("Mirroring I2S samples to %s", mgos_sys_config_get_app_i2s_sample_mirror()));
    }
  }

  // Start the receiver thread
  xTaskCreate(i2s_scanner, "i2s_scanner", 1024 * 10, NULL, tskIDLE_PRIORITY, NULL);

  // Start reporting timer
  mgos_set_timer(5000, true, i2c_scanner_stats, NULL);

  // Register Prometheus handler
  mgos_prometheus_metrics_add_handler(prometheus_metrics_fn, NULL);
}
