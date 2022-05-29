
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ds18x20.h>
#include <bmp280.h>
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
static ds18x20_addr_t addrs[MAX_SENSORS];
static int sensor_count = 0;
static float temps[MAX_SENSORS];

static int ow_scan_devices(gpio_num_t pin, ds18x20_addr_t *addr_list, int addr_count)
{
    //CHECK_ARG(addr_list);

    onewire_search_t search;
    onewire_addr_t addr;
    int found = 0;

    onewire_search_start(&search);
    while ((addr = onewire_search_next(&search, pin)) != ONEWIRE_NONE)
    {
        uint8_t family_id = (uint8_t)addr;
        printf("found: %d\n", (int)family_id);
        if (found < addr_count)
            addr_list[found] = addr;
        found++;

    }
    return found;
}
static void init_hw(void)
{
    i2cdev_init();

    memset(&temp_sensor, 0, sizeof(bmp280_t));
    temp_sensor.i2c_dev.timeout_ticks = 0xffff / portTICK_PERIOD_MS;

    bmp280_params_t params;
    bmp280_init_default_params(&params);
//BMP280_I2C_ADDRESS_0
    bmp280_init_desc(&temp_sensor, 0x77, 0, SDA_GPIO, SCL_GPIO);
    bmp280_init(&temp_sensor, &params);

    // onewire

    while (sensor_count == 0)
    {
        sensor_count = ds18x20_scan_devices((gpio_num_t)OW_SENSOR_PIN, addrs, MAX_SENSORS);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    if (sensor_count > MAX_SENSORS)
    {
        sensor_count = MAX_SENSORS;
    }
}


void readSensors() {
    float pressure, temperature, humidity;
    while (1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        if (bmp280_read_float(&temp_sensor, &temperature, &pressure, &humidity) == ESP_OK)
        {
            printf("%.2f Pa, %.2f C, %.2f %%\n", pressure, temperature, humidity);
        }

        ds18x20_measure_and_read_multi((gpio_num_t)OW_SENSOR_PIN, addrs, sensor_count, temps);
        for (int i = 0; i < sensor_count; i++)
        {
            printf("sensor-id: %08x temp: %fC\n", (uint32_t)addrs[i], temps[i]);
        }
    }
}


static void sync_time(void *arg)
{
    printf("running sntp sync.. \n");

    int retry = 0;
    const int retry_count = 10;

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        printf(".");
    }
    printf("\n");
    if (retry == retry_count)
    {
      
        printf("Sync failed.");
        vTaskDelete(NULL);
        return;
    }

    printf("Sync successfull, ");
    time_t now = 0;
    struct tm timeinfo;
    memset((void *)&timeinfo, 0, sizeof(timeinfo));
    char buf[64];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
    printf("time: %s\n", buf);
    vTaskDelete(NULL);
}

static bool xTimeInitialized = false;
static void prvTimeSyncNotificationCallback( struct timeval * pxTimeVal )
{
    ( void ) pxTimeVal;
    ESP_LOGI( TAG, "Notification of a time synchronization event" );
    xTimeInitialized = true;

     printf("Sync successfull, ");
    time_t now = 0;
    struct tm timeinfo;
    memset((void *)&timeinfo, 0, sizeof(timeinfo));
    char buf[64];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
    printf("time: %s\n", buf);
}

static void handle_wifi_connect(void)
{
    //xTaskCreate(connect_aws_mqtt, "connect_aws_mqtt", 15 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    //apptemp_init(publish_reading);
    
    //readSensors();
    printf("wifi connected...");

    sntp_setoperatingmode( SNTP_OPMODE_POLL );
    sntp_set_time_sync_notification_cb( prvTimeSyncNotificationCallback );
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    int retry_count = 0;
    while (!xTimeInitialized)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        printf(".");
    }

}

static void handle_wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}


void app_main()
{
    connect_wifi_params_t cbs = {
        .on_connected = handle_wifi_connect,
        .on_failed = handle_wifi_failed};
    appwifi_connect(cbs);
}
