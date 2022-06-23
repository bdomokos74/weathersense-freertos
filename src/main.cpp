

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "bmp280.h"
#include <string.h>
#include "esp_log.h"

#include "iot.h"
#include "app_wifi.h"
#include "telemetry.h"

extern "C" void app_main(void);

#define SDA_GPIO 21
#define SCL_GPIO 22

#define TAG "app"

#define OW_SENSOR_PIN 19
#define MAX_SENSORS 8
#define BME_ADDR 0x77


// static ds18x20_addr_t addrs[MAX_SENSORS];
static bmp280_t temp_sensor;
static int sensor_count = 0;
//static float temps[MAX_SENSORS];


float temperature;
float pressure;
float humidity;

#define IOT_CONFIG_IOTHUB_FQDN "weathersensehub.azure-devices.net"
#define IOT_CONFIG_DEVICE_ID (char*)"DOIT2"
#define IOT_CONFIG_DEVICE_KEY (char*)"imIMZOyVc1PRqVvrSg587PLwIWkJ23AX3zVxjOVg5m8="

char *iothubHost = IOT_CONFIG_IOTHUB_FQDN;
char *mqtt_broker_uri = "mqtts://" IOT_CONFIG_IOTHUB_FQDN;
char *iotDeviceId = IOT_CONFIG_DEVICE_ID;
char *iotDeviceKey = IOT_CONFIG_DEVICE_KEY;


static void telemetryTask(void *arg)
{
    telemetryInit(SDA_GPIO, SCL_GPIO);
    while (1)
    {
        sendTelemetry();
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

static void on_timeset() {
    
    xTaskCreate(telemetryTask, TAG, 3 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);

    initializeIoTHubClient();
    
    initializeMqttClient();
}

static void on_connected() {
    ESP_LOGI(TAG, "on_connected");
}

static void on_failed() {
    ESP_LOGI(TAG, "on_failed");
}

void app_main()
{
    connect_wifi_params_t params = {
        .on_connected = on_connected,
        .on_failed = on_failed,
        .on_timeset = on_timeset
    };
    appwifi_connect(params);
}
