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

extern TSafeVars mainTSafeVars;

#define TAG "SLPMODE"

static BME280Sensor *bme280Sensor;

int trySendingTelemetry(Telemetry *telemetry);

static char telemetrySleepTaskName[] = "TMSTASK";

static Props props;

#define RTC_BUF_SIZE 3072
extern char dataBuf[];
extern char *bufPoi;
extern int numStored;
extern int telemetryId;

static Props currProps;
static void telemetryTaskSleepmode(void *arg)
{
    Telemetry *telemetry = new Telemetry(
        (char*)dataBuf,
        RTC_BUF_SIZE,
        bufPoi,
        &numStored,
        &telemetryId
    );
    ESP_LOGI(telemetrySleepTaskName, "started");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    bool succ = false;
    int twinSucc = 0;
    bool twinPatchSucc = false;
    int retry = 20;
    requestTwin();
    while((!succ||(twinSucc!=2))&&retry>0) {
        retry--;
        if(!succ) {
            succ = (WSOK==trySendingTelemetry(telemetry));
        }
        if(twinSucc==0) {
            if(mainTSafeVars.getAndClearTwinGetSuccess()) {
                twinSucc++;
                sendTwinProp();
            }
        }  else if(twinSucc == 1) {
            if(mainTSafeVars.getAndClearTwinPatchSuccess()) {
                twinSucc++;
            }
        }
        
        if(succ&&(twinSucc==2)) {
           break;
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    if(!succ) {
        ESP_LOGI(telemetrySleepTaskName, "Failed to send telemetry");
    }
    if(twinSucc!=2) {
        ESP_LOGI(telemetrySleepTaskName, "Failed twin req/patch");
    }
    mqttClientDisconnect();
    mqttClientDestroy();
    goSleep(mS_TO_S_FACTOR * props.getMeasureIntervalMs());
}

void logTelemetryStatus(Telemetry *telemetry);

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

static volatile BaseType_t ttask = NULL;
static void on_timeset_sleepmode() {
    uint32_t stackSize = 3 * configMINIMAL_STACK_SIZE;
    ESP_LOGI(TAG, "Creating TMSTASK, stackSize=%d", stackSize);
    if(ttask==NULL) {
        ttask = xTaskCreate(telemetryTaskSleepmode, telemetrySleepTaskName, stackSize, NULL, 5, NULL);
    }
    //mqttClientDestroy();
    initializeIoTHubClient();
    initializeMqttClient();

}

static unsigned hwm = 0;
void buildConnectionSleepMode() {
    connect_wifi_params_t params = {
        .on_connected = on_connected,
        .on_disconnected = on_disconnected,
        .on_failed = on_failed,
        .on_timeset = on_timeset_sleepmode
    };
    hwm = logHWMIfHigher(hwm);

    appwifi_connect(params);

    ESP_LOGI(TAG, "after esp_wifi_start");
}

#define RTC_BUF_SIZE 3072
extern char dataBuf[];
extern char *bufPoi;
extern int numStored;
extern int telemetryId;

static uint8_t telemetry_payload[100];
az_span sleepModeMeas;
void runInSleepMode() {
    Telemetry *telemetry = new Telemetry(
        (char*)dataBuf,
        RTC_BUF_SIZE,
        bufPoi,
        &numStored,
        &telemetryId
    );
    Props::load(props);
    props.debug("INITIAL props(sleep)");

    float temperature;
    float pressure;
    float humidity;
    logTelemetryStatus(telemetry);
    bme280Sensor = new BME280Sensor(SDA_GPIO, SCL_GPIO);
    if(bme280Sensor->readMeasurement(temperature, pressure, humidity)==WSOK) 
    {
        sleepModeMeas = AZ_SPAN_FROM_BUFFER(telemetry_payload);
        telemetry->buildTelemetryPayload(sleepModeMeas, &sleepModeMeas, temperature, pressure, humidity);
        logSpan(TAG, "telemetry1", sleepModeMeas);
        logTelemetryStatus(telemetry);
        if(telemetry->getNumStored()>=props.getMeasureBatchSize() || !telemetry->doesMeasurementFit(sleepModeMeas)) {
            ESP_LOGI(TAG, "connecting to send measurements");
            buildConnectionSleepMode();
        } else {
            ESP_LOGI(TAG, "storing telemetry");
            telemetry->storeMeasurement(sleepModeMeas);
            logTelemetryStatus(telemetry);
            goSleep(mS_TO_S_FACTOR * props.getMeasureIntervalMs());
        }
    } else {
        ESP_LOGI(TAG, "sensor not found");
        goSleep(mS_TO_S_FACTOR * props.getMeasureIntervalMs());
    }
    ESP_LOGI(TAG, "runInSleepMode - exit");
    while(true) {
        logHWM();
        ESP_LOGI(TAG, "delay..");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
