

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "bmp280.h"
#include <string.h>
#include "esp_log.h"

#include "iot.h"
#include "app_wifi.h"
#include "telemetry.h"
#include "bme280sensor.h"

#include "props.h"

extern "C" void app_main(void);

#define SDA_GPIO 21
#define SCL_GPIO 22

#define TAG "MAIN"

#define OW_SENSOR_PIN 19
#define MAX_SENSORS 8
#define BME_ADDR 0x77

// static ds18x20_addr_t addrs[MAX_SENSORS];
//static bmp280_t temp_sensor;
static int sensor_count = 0;
//static float temps[MAX_SENSORS];

BME280Sensor *bme280Sensor;

char *iothubHost = IOT_CONFIG_IOTHUB_FQDN;
char *mqtt_broker_uri = "mqtts://" IOT_CONFIG_IOTHUB_FQDN;
char *iotDeviceId = IOT_CONFIG_DEVICE_ID;
char *iotDeviceKey = IOT_CONFIG_DEVICE_KEY;

static void telemetryTask(void *arg)
{
    bme280Sensor = new BME280Sensor(SDA_GPIO, SCL_GPIO);
    static uint8_t telemetry_payload[100];
    float temperature;
    float pressure;
    float humidity;
    while (1)
    {
        bme280Sensor->readMeasurement(temperature, pressure, humidity);
        az_span telemetry = AZ_SPAN_FROM_BUFFER(telemetry_payload);
        getTelemetryPayload(telemetry, &telemetry, temperature, pressure, humidity);
        sendTelemetry(telemetry);

        Props props;
        Props::load(props);
        int delayMs = props.getMeasureIntervalMs();
        ESP_LOGI(TAG, "delay: %d", delayMs);
        vTaskDelay(delayMs / portTICK_PERIOD_MS);
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
    Props::init();
    connect_wifi_params_t params = {
        .on_connected = on_connected,
        .on_failed = on_failed,
        .on_timeset = on_timeset
    };
    appwifi_connect(params);
}
