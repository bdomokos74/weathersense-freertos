
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
//#include <ds18x20.h>
#include "bmp280.h"
#include <string.h>
#include "esp_log.h"
#include "app_wifi.h"
#include "esp_sntp.h"

#define SDA_GPIO 21
#define SCL_GPIO 22

static bmp280_t temp_sensor;

#define TAG "app"

#define OW_SENSOR_PIN 19
#define MAX_SENSORS 8
// static ds18x20_addr_t addrs[MAX_SENSORS];
static int sensor_count = 0;
static float temps[MAX_SENSORS];

#define BME_ADDR 0x77

// BMESensor *bmeSensor;
float temperature;
float pressure;
float humidity;

static void init_hw(void)
{
    i2cdev_init();

    memset(&temp_sensor, 0, sizeof(bmp280_t));
    temp_sensor.i2c_dev.timeout_ticks = 0xffff / portTICK_PERIOD_MS;

    bmp280_params_t params;
    bmp280_init_default_params(&params);
    // BMP280_I2C_ADDRESS_0
    bmp280_init_desc(&temp_sensor, 0x77, 0, SDA_GPIO, SCL_GPIO);
    bmp280_init(&temp_sensor, &params);

    if (bmp280_read_float(&temp_sensor, &temperature, &pressure, &humidity) == ESP_OK)
    {
        ESP_LOGI(TAG, "%.2f Pa, %.2f C, %.2f %%\n", pressure, temperature, humidity);
    }
    // bmeSensor = new BMESensor(BME_ADD);
}

// void initConn() {
//     initializeIoTHubClient();
//     initializeMqttClient();
// }
void readTemp()
{
    while (1)
    {
        if (bmp280_read_float(&temp_sensor, &temperature, &pressure, &humidity) == ESP_OK)
        {
            ESP_LOGI(TAG, "%.2f Pa, %.2f C, %.2f %%\n", pressure, temperature, humidity);
        }
        else
        {
            ESP_LOGI(TAG, "temp failed");
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

static bool xTimeInitialized = false;
static void prvTimeSyncNotificationCallback(struct timeval *pxTimeVal)
{
    (void)pxTimeVal;
    ESP_LOGI(TAG, "Notification of a time synchronization event");
    xTimeInitialized = true;

    
    time_t now = 0;
    struct tm timeinfo;
    memset((void *)&timeinfo, 0, sizeof(timeinfo));
    char buf[64];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "Sync successfull, time: %s", buf);

    xTaskCreate(readTemp, TAG, 3 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}

static void handle_wifi_connect(void)
{
    // xTaskCreate(connect_aws_mqtt, "connect_aws_mqtt", 15 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    // apptemp_init(publish_reading);

    // readSensors();
    ESP_LOGI(TAG, "wifi connected...\n");

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_time_sync_notification_cb(prvTimeSyncNotificationCallback);
    sntp_setservername(0, "pool.ntp.org");
    //"time.nist.gov"
    sntp_init();

    int retry_count = 0;
    while (!xTimeInitialized)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, ".");
    }
}

static void handle_wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main()
{
    init_hw();

    connect_wifi_params_t cbs = {
        .on_connected = handle_wifi_connect,
        .on_failed = handle_wifi_failed};
    appwifi_connect(cbs);
}
