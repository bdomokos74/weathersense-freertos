
#include "app_wifi.h"

#define TAG "appwifi"

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_sntp.h"

#include "app_wifi.h"
#include "logger.h"

static connect_wifi_params_t m_params;


#define MAX_RETRY 10
static int retry_cnt = 0;

static bool xTimeInitialized = false;
static void prvTimeSyncNotificationCallback(struct timeval *pxTimeVal)
{
    logHWM();
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
    if(m_params.on_timeset) {
        m_params.on_timeset();
    }
    logHWM();
}

static void got_ip_cb(void)
{
    logHWM();
    // xTaskCreate(connect_aws_mqtt, "connect_aws_mqtt", 15 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    // apptemp_init(publish_reading);

    if (m_params.on_connected)
    {
        m_params.on_connected();
    }

    // readSensors();
    ESP_LOGI(TAG, "wifi connected...\n");
    

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_time_sync_notification_cb(prvTimeSyncNotificationCallback);
    sntp_setservername(0, "pool.ntp.org");
    //"time.nist.gov"
    sntp_init();

    while (!xTimeInitialized)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    logHWM();
}

static void handle_wifi_connection(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retry_cnt++ < MAX_RETRY)
        {
            esp_wifi_connect();
        }
        else
        {
            if (m_params.on_failed)
            {
                m_params.on_failed();
            }
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        if (m_params.on_connected)
        {
            m_params.on_connected();
        }
        got_ip_cb();
    }
}

void appwifi_connect(connect_wifi_params_t p)
{

    m_params = p;

    if (nvs_flash_init() != ESP_OK)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_event_loop_create_default();
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &handle_wifi_connection, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &handle_wifi_connection, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold = {.authmode = WIFI_AUTH_WPA2_PSK},
            //.scan_method = WIFI_ALL_CHANNEL_SCAN,
            //.sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            //.threshold.rssi = -127,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };

    esp_netif_init();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    logHWM();
}
