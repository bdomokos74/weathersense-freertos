

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "bmp280.h"
#include <string.h>
#include "esp_log.h"
#include "Logger.h"

#include "iot.h"
#include "app_wifi.h"
#include "telemetry.h"
#include "bme280sensor.h"

#include "props.h"
#include "wake.h"

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



char *iothubHost = IOT_CONFIG_IOTHUB_FQDN;
char *mqtt_broker_uri = "mqtts://" IOT_CONFIG_IOTHUB_FQDN;
char *iotDeviceId = IOT_CONFIG_DEVICE_ID;
char *iotDeviceKey = IOT_CONFIG_DEVICE_KEY;

#define RTC_BUF_SIZE 3072
RTC_DATA_ATTR char dataBuf[RTC_BUF_SIZE];
RTC_DATA_ATTR char *bufPoi = dataBuf;
RTC_DATA_ATTR int numStored = 0;

BME280Sensor *bme280Sensor;
Telemetry *telemetry;
static Props props;
void logTelemetryStatus(Telemetry *telemetry) {
    char statusBuf[200];
    telemetry->buildStatus(statusBuf, sizeof(statusBuf));
    ESP_LOGI(TAG, "%s", statusBuf);
}

static uint8_t telemetry_payload[100];
static float temperature;
static float pressure;
static float humidity;
static char telemetryTaskName[] = "TMTASK";
static unsigned ttHwm = 0;
static void telemetryTask(void *arg)
{
    
    bme280Sensor = new BME280Sensor(SDA_GPIO, SCL_GPIO);

    while (1)
    {
        Props::load(props);
        bme280Sensor->readMeasurement(temperature, pressure, humidity);
        az_span meas = AZ_SPAN_FROM_BUFFER(telemetry_payload);
        telemetry->buildTelemetryPayload(meas, &meas, temperature, pressure, humidity);

        if(!telemetry->doesMeasurementFit(meas)||numStored>=props.getMeasureBatchSize()) {
            if(sendTelemetry(az_span_create((uint8_t*)dataBuf, telemetry->getStoredSize()))==0) {
                ESP_LOGI(telemetryTaskName, "Failed to send telemetry data");
            } else {
                telemetry->reset();
            } 
        } else {
            logSpan(telemetryTaskName, "telemetry", meas);
            telemetry->storeMeasurement(meas);
            logTelemetryStatus(telemetry);
        }
        
        int delayMs = props.getMeasureIntervalMs();
        ttHwm = logHWMIfHigher(telemetryTaskName, ttHwm);
        ESP_LOGI(telemetryTaskName, "delay: %d ms", delayMs);
        vTaskDelay(delayMs / portTICK_PERIOD_MS);
    }
}

static void on_timeset() {
    uint32_t stackSize = 3 * configMINIMAL_STACK_SIZE;
    ESP_LOGI(TAG, "Creating TMTASK, stackSize=%d", stackSize);
    xTaskCreate(telemetryTask, "TMTASK", stackSize, NULL, 5, NULL);

    initializeIoTHubClient();
    
    initializeMqttClient();
}

static void on_connected() {
    ESP_LOGI(TAG, "on_connected");
}

static void on_failed() {
    ESP_LOGI(TAG, "on_failed");
}

void c2d_info_handler(void) {
    logWakeReason();
    logTelemetryStatus(telemetry);
}

static unsigned hwm = 0;
void app_main()
{
    logWakeReason();

    telemetry = new Telemetry(
        (char*)dataBuf,
        RTC_BUF_SIZE,
        &bufPoi,
        &numStored
    );

    Props::init();
    connect_wifi_params_t params = {
        .on_connected = on_connected,
        .on_failed = on_failed,
        .on_timeset = on_timeset
    };
    hwm = logHWMIfHigher(TAG, hwm);
    appwifi_connect(params);

    ESP_LOGI(TAG, "after esp_wifi_start");
    while(true) {
        hwm = logHWMIfHigher(TAG, hwm);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
