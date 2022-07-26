#include "iot.h"
#include <az_core.h>
#include <az_iot.h>
#include "esp_log.h"
#include <Logger.h>
#include <mqtt_client.h>
#include "wake.h"

#define TAG "C2D"

extern az_iot_hub_client client;

extern void c2d_info_handler(void);

bool handleC2d(esp_mqtt_event_handle_t event)
{
    az_iot_hub_client_c2d_request response;
    if (az_result_failed(az_iot_hub_client_c2d_parse_received_topic(&client, az_span_create((uint8_t *)event->topic, event->topic_len), &response)))
    {
        ESP_LOGI(TAG, "not c2d - %d", event->event_id);
        return false;
    }

    //az_span props = response.properties._internal.properties_buffer;
    //2022/6/5 23:18:18 c2d content:%24.to=%2Fdevices%2FDOIT2%2Fmessages%2FdeviceBound&%24.ct=text%2Fplain%3B%20charset%3DUTF-8&%24.ce=utf-8

    logBuf(TAG, "c2d content:", (char*)event->data, event->data_len);

    if(az_span_is_content_equal(az_span_create_from_str("info"), az_span_create((uint8_t*)event->data, event->data_len))) {
        c2d_info_handler();
    }
    return true;
}
