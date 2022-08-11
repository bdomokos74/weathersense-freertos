

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
#include "dallas_sensor.h"
#include "wscommon.h"

#include "props.h"
#include "tsafevars.h"
#include "wake.h"

#include "wsota.h"
#include "esp_ota_ops.h"
#include "esp_system.h"

extern "C" void app_main(void);

#define TAG "MAIN"

//#define OW_SENSOR_PIN 19
//#define MAX_SENSORS 8
//#define BME_ADDR 0x76


// static ds18x20_addr_t addrs[MAX_SENSORS];
//static bmp280_t temp_sensor;
static int sensor_count = 0;
//static float temps[MAX_SENSORS];

char *iothubHost = IOT_CONFIG_IOTHUB_FQDN;
char *mqtt_broker_uri = "mqtts://" IOT_CONFIG_IOTHUB_FQDN;
char *iotDeviceId = IOT_CONFIG_DEVICE_ID;
char *iotDeviceKey = IOT_CONFIG_DEVICE_KEY;


extern char dataBuf[];
extern int rtcBufSize;
extern int bytesStored;
extern int numStored;
extern int telemetryId;

BME280Sensor *bme280Sensor0;
BME280Sensor *bme280Sensor1;
DallasSensor *dallasSensor;
static Props props;

TSafeVars mainTSafeVars;

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
    ESP_LOGI(telemetryTaskName, "started, dataBuf=%p, bytesStored=%d, numStored=%d, telemetryId=%d", dataBuf, bytesStored, numStored, telemetryId);
    Telemetry *telemetry = new Telemetry(
        (char*)dataBuf,
        rtcBufSize,
        &bytesStored,
        &numStored,
        &telemetryId
    );
    ESP_LOGI(TAG, "INITIAL telemetry store:");
    
    bme280Sensor0 = new BME280Sensor(SDA_GPIO, SCL_GPIO, BMP280_I2C_ADDRESS_0);
    bme280Sensor1 = new BME280Sensor(SDA_GPIO, SCL_GPIO, BMP280_I2C_ADDRESS_1);
    dallasSensor = new DallasSensor(ONE_W_PIN);
    telemetry->addSensor(bme280Sensor0);
    telemetry->addSensor(bme280Sensor1);
    telemetry->addSensor(dallasSensor);

    esp_reset_reason_t resetReason = esp_reset_reason();
    ESP_LOGI(telemetryTaskName, "Reset Reason: %d", resetReason);
            
    bool twinGetDone = false;
    while (1)
    {
        ESP_LOGI(telemetryTaskName, "loop");
        Props::load(props);

        if(!twinGetDone &&resetReason!= ESP_RST_TASK_WDT && mainTSafeVars.getAndClearTwinGetSubscribed()) {
            twinGetDone = true;
            ESP_LOGI(telemetryTaskName, "Requesting twin desired");
            requestTwin();
        }

        az_span meas = AZ_SPAN_FROM_BUFFER(telemetry_payload);
        if(telemetry->buildTelemetryPayload(meas, &meas)) {

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
                if(trySendingTelemetry(telemetry)==WSOK) {
                    esp_task_wdt_reset();
                };
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
            while(!mainTSafeVars.getAndClearDisconnected()) {
                 ESP_LOGI(telemetryTaskName, "waiting to disconnect");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            ESP_LOGI(telemetryTaskName, "disconnect done");
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
   // mqttClientDestroy();
}

void c2d_info_handler(void) {
    uint8_t macBuf[6];
    logWakeReason();
    //logTelemetryStatus(telemetry);
    esp_read_mac(macBuf, ESP_MAC_WIFI_STA);
    hexDump("MAC", macBuf, sizeof(macBuf), 16);

    const esp_app_desc_t *app_desc = esp_ota_get_app_description();
    ESP_LOGI(TAG, "Application information:");
    ESP_LOGI(TAG, "GIT revision:     %s", props.getGitRevision());
    ESP_LOGI(TAG, "Project name:     %s", app_desc->project_name);
    ESP_LOGI(TAG, "App version:      %s", app_desc->version);
    ESP_LOGI(TAG, "Compile time:     %s %s", app_desc->date, app_desc->time);
    char buf[17];
    esp_ota_get_app_elf_sha256(buf, sizeof(buf));
    ESP_LOGI(TAG, "ELF file SHA256:  %s...", buf);
    ESP_LOGI(TAG, "ESP-IDF:          %s", app_desc->idf_ver);
    ESP_LOGI(TAG, "DEVICE_ID:        %s", iotDeviceId);
}

void cm_info_handler(void) {
}

void runInSleepMode();

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

    esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
    esp_task_wdt_add(NULL);    

    i2cdev_init();

    Props::init();
    Props::load(props);

    if(props.getDoSleep()>0) {
        ESP_LOGI(TAG, "sleep mode selected");
        runInSleepMode();
    } else {
        ESP_LOGI(TAG, "conn mode selected, databuf=%p, numStored=%d", dataBuf, numStored);
        props.debug("INITIAL"); 
        numStored = 0;
        telemetryId = 0;
        runInConnectedMode();
    }
}