#include "telemetry.h"
#include "esp_log.h"
#include "bmp280.h"
#include <time.h>

#define TAG "telemetry"

static bmp280_t temp_sensor;
static int sensor_count = 0;


static float temperature;
static float pressure;
static float humidity;

static uint32_t telemetry_send_count = 0;

void telemetryInit(int sda, int scl) {
    ESP_LOGI(TAG, "telemetry init");
    i2cdev_init();

    memset(&temp_sensor, 0, sizeof(bmp280_t));
    temp_sensor.i2c_dev.timeout_ticks = 0xffff / portTICK_PERIOD_MS;

    bmp280_params_t params;
    bmp280_init_default_params(&params);
    // BMP280_I2C_ADDRESS_0
    bmp280_init_desc(&temp_sensor, 0x77, 0,  (gpio_num_t)sda,  (gpio_num_t)scl);
    bmp280_init(&temp_sensor, &params);
    ESP_LOGI(TAG, "telemetry init done");
}

void getTelemetryPayload(az_span payload, az_span* out_payload)
{
  char tempBuf[20];
  
  
  if (bmp280_read_float(&temp_sensor, &temperature, &pressure, &humidity) == ESP_OK)
  {
      ESP_LOGI(TAG, "%.2f Pa, %.2f C, %.2f %%\n", pressure, temperature, humidity);
  } else {
      ESP_LOGE(TAG, "sensor read failed");
  }
  sprintf(tempBuf, "%.2f", temperature);
  az_span tempSpan = az_span_create_from_str(tempBuf);

  char pBuf[20];
  sprintf(pBuf, "%.2f", pressure);
  az_span pSpan = az_span_create_from_str(pBuf);

  char hBuf[20];
  sprintf(hBuf, "%.2f", humidity);
  az_span hSpan = az_span_create_from_str(hBuf);

  // --------
  az_span original_payload = payload;

  payload = az_span_copy(payload, AZ_SPAN_FROM_STR("{\"id\":"));
  (void)az_span_u32toa(payload, telemetry_send_count++, &payload);

  payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"ts\":"));
  time_t now = time(NULL);
  (void)az_span_u64toa(payload, now, &payload);
  
  payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"t1\":"));
  payload = az_span_copy(payload, tempSpan);
  payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"p\":"));
  payload = az_span_copy(payload, pSpan);
  payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"h\":"));
  payload = az_span_copy(payload, hSpan);

  payload = az_span_copy(payload, AZ_SPAN_FROM_STR("}"));
  payload = az_span_copy_u8(payload, '\0');
  
  *out_payload = az_span_slice(original_payload, 0, az_span_size(original_payload) - az_span_size(payload) - 1);
  
  ESP_LOGI(TAG, "payload = %s", (char*)az_span_ptr(*out_payload));
}