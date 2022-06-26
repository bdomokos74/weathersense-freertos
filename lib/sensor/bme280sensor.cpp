#include "bme280sensor.h"
#include "wscommon.h"

#include "esp_log.h"

#define TAG "BME280"
#include "bmp280.h"

#include "string.h"
#include <time.h>

BME280Sensor::BME280Sensor(int sda, int scl) {
    ESP_LOGI(TAG, "init");
    i2cdev_init();

    memset(&this->temp_sensor, 0, sizeof(bmp280_t));
    temp_sensor.i2c_dev.timeout_ticks = 0xffff / portTICK_PERIOD_MS;

    bmp280_params_t params;
    bmp280_init_default_params(&params);
    // BMP280_I2C_ADDRESS_0
    bmp280_init_desc(&temp_sensor, 0x77, 0,  (gpio_num_t)sda,  (gpio_num_t)scl);
    bmp280_init(&temp_sensor, &params);

    ESP_LOGI(TAG, "init done");
}

int BME280Sensor::readMeasurement(float &temperature, float &pressure, float &humidity) {
    float temp, pres, hum;
    if (bmp280_read_float(&this->temp_sensor, &temp, &pres, &hum) == ESP_OK)
    {
        ESP_LOGI(TAG, "%.2f Pa, %.2f C, %.2f %%", pres, temp, hum);
    } else {
        ESP_LOGE(TAG, "sensor read failed");
        return WSNOK;
    }
    temperature = temp;
    pressure = pres;
    humidity = hum;
    return WSOK;
}

