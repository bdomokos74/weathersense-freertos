

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "bmp280.h"
#include <string.h>
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_sleep.h"

#include "Logger.h"

#include "iot.h"
#include "app_wifi.h"
#include "telemetry.h"
#include "bme280sensor.h"
#include "wscommon.h"

#include "props.h"
#include "tsafevars.h"
#include "wake.h"

extern "C" void app_main(void);

#define TAG "MAIN"

#define OW_SENSOR_PIN 19
#define MAX_SENSORS 8
#define BME_ADDR 0x77


// static ds18x20_addr_t addrs[MAX_SENSORS];
//static bmp280_t temp_sensor;
static int sensor_count = 0;
//static float temps[MAX_SENSORS];

#define WDT_TIMEOUT_SEC 60*60

char *iothubHost = IOT_CONFIG_IOTHUB_FQDN;
char *mqtt_broker_uri = "mqtts://" IOT_CONFIG_IOTHUB_FQDN;
char *iotDeviceId = IOT_CONFIG_DEVICE_ID;
char *iotDeviceKey = IOT_CONFIG_DEVICE_KEY;

#define RTC_BUF_SIZE 3072
extern char dataBuf[];
extern char *bufPoi;
extern int numStored;
extern int telemetryId;

BME280Sensor *bme280Sensor;
static Props props;

void logTelemetryStatus(Telemetry *telemetry) {
    char statusBuf[200];
    telemetry->buildStatus(statusBuf, sizeof(statusBuf));
    ESP_LOGI(TAG, "%s", statusBuf);
}

static uint8_t telemetry_payload[100];
static char telemetryTaskName[] = "TMTASK";
static unsigned ttHwm = 0;
int trySendingTelemetry(Telemetry *telemetry) {
    ESP_LOGI(telemetryTaskName, "try sending telemetry");
    if(sendTelemetry(az_span_create((uint8_t*)telemetry->getDataBuf(), telemetry->getStoredSize()))==0) {
        ESP_LOGI(telemetryTaskName, "Failed to send telemetry data");
        return WSNOK;
    } else {
        ESP_LOGI(telemetryTaskName, "Stored telemetry sent out");
        telemetry->reset();
        return WSOK;
    } 
}
static void telemetryTask(void *arg)
{
    ESP_LOGI(telemetryTaskName, "started, dataBuf=%p, bufpoi=%p, numStored=%p, telemetryId=%p", dataBuf, bufPoi, &numStored, &telemetryId);
    Telemetry *telemetry = new Telemetry(
        (char*)dataBuf,
        RTC_BUF_SIZE,
        (char*)bufPoi,
        &numStored,
        &telemetryId
    );
    ESP_LOGI(TAG, "INITIAL telemetry store:");
    
    esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
    esp_task_wdt_add(NULL);    
    bme280Sensor = new BME280Sensor(SDA_GPIO, SCL_GPIO);
    float temperature;
    float pressure;
    float humidity;
    while (1)
    {
        ESP_LOGI(telemetryTaskName, "loop");
        Props::load(props);
        if(bme280Sensor->readMeasurement(temperature, pressure, humidity)==WSOK) 
        {
            az_span meas = AZ_SPAN_FROM_BUFFER(telemetry_payload);
            telemetry->buildTelemetryPayload(meas, &meas, temperature, pressure, humidity);

            if(telemetry->getNumStored()>=props.getMeasureBatchSize()) {
                if(trySendingTelemetry(telemetry)==WSOK) {
                    esp_task_wdt_reset();
                };
            }
            logSpan(telemetryTaskName, "telemetry", meas);
            if(telemetry->doesMeasurementFit(meas)) {
                 ESP_LOGI(telemetryTaskName, "storing measurement");
                telemetry->storeMeasurement(meas);
                ESP_LOGI(telemetryTaskName, "done stoing measurement");
            } else {
                trySendingTelemetry(telemetry);
                if(telemetry->doesMeasurementFit(meas)) {
                    telemetry->storeMeasurement(meas);
                } else {
                    ESP_LOGE(telemetryTaskName, "Can't store telemetry, dropping it...");
                }
            }
            logTelemetryStatus(telemetry);
        } else {
            ESP_LOGE(telemetryTaskName, "Sensor read failed, skip telemetry");
        }
        int delayMs = props.getMeasureIntervalMs();
        ttHwm = logHWMIfHigher(ttHwm);
        ESP_LOGI(telemetryTaskName, "delay: %d ms", delayMs);

        if(props.getDoSleep()>0) {
            mqttClientDisconnect();
            //vTaskDelay(1000 / portTICK_PERIOD_MS);
            mqttClientDestroy();
            goSleep(mS_TO_S_FACTOR * props.getMeasureIntervalMs());
        }
        vTaskDelay(delayMs / portTICK_PERIOD_MS);
    }
}

static volatile BaseType_t ttask = NULL;
static void on_timeset() {
    uint32_t stackSize = 3 * configMINIMAL_STACK_SIZE;
    ESP_LOGI(TAG, "Creating TMTASK, stackSize=%d", stackSize);
    if(ttask==NULL) {
        ttask = xTaskCreate(telemetryTask, telemetryTaskName, stackSize, NULL, 5, NULL);
    }
    mqttClientDestroy();
    initializeIoTHubClient();
    initializeMqttClient();
}

static void on_connected() {
    ESP_LOGI(TAG, "on_connected");
}

static void on_failed() {
    ESP_LOGI(TAG, "on_failed");
    goSleep(uS_TO_S_FACTOR * WIFI_FAIL_SLEEP_TIME_SEC);
}

static void on_disconnected() {
    ESP_LOGI(TAG, "on_disconnected");
    mqttClientDestroy();
}

void c2d_info_handler(void) {
    uint8_t macBuf[6];
    logWakeReason();
    //logTelemetryStatus(telemetry);
    esp_read_mac(macBuf, ESP_MAC_WIFI_STA);
    hexDump("MAC", macBuf, sizeof(macBuf), 16);
}


void runInSleepMode();

TSafeVars mainTSafeVars;
static unsigned hwm = 0;
void runInConnectedMode() {
    connect_wifi_params_t params = {
        .on_connected = on_connected,
        .on_disconnected = on_disconnected,
        .on_failed = on_failed,
        .on_timeset = on_timeset
    };
    hwm = logHWMIfHigher(hwm);

    appwifi_connect(params);

    ESP_LOGI(TAG, "after esp_wifi_start");
    while(true) {
        hwm = logHWMIfHigher(hwm);

        // when the sas token expires...
        if(mainTSafeVars.getAndClearMqttRestartReqested()) {
            mqttClientDestroy();
            initializeIoTHubClient();
            initializeMqttClient();
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    ESP_LOGI(TAG, "app_main called");
    logWakeReason();


    Props::init();
    Props::load(props);

    if(props.getDoSleep()>0) {
        ESP_LOGI(TAG, "sleep mode selected");
        runInSleepMode();
    } else {
        ESP_LOGI(TAG, "conn mode selected, databuf=%p, bufpoi=%p", dataBuf, bufPoi);
        props.debug("INITIAL");
        bufPoi = (char*)(dataBuf);
        ESP_LOGI(TAG, "conn mode selected, databuf=%p, bufpoi=%p", dataBuf, bufPoi);
        numStored = 0;
        telemetryId = 0;
        runInConnectedMode();
    }
}