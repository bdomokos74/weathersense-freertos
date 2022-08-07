#include "iot.h"
#include <az_core.h>
#include <az_iot.h>
#include "esp_log.h"
#include <Logger.h>
#include <mqtt_client.h>
#include "wake.h"
#include "props.h"

#define TAG "CM"

extern az_iot_hub_client client;
extern void cm_info_handler(void);

extern esp_mqtt_client_handle_t mqtt_client;

static bool _sendMethodResponse(az_iot_hub_client_method_request *method_request, az_iot_status status, az_span response)
{
    char methods_response_topic_buffer[200];
    if (az_result_failed(az_iot_hub_client_methods_response_get_publish_topic(
            &client,
            method_request->request_id,
            (uint16_t)status,
            methods_response_topic_buffer,
            sizeof(methods_response_topic_buffer),
            NULL)))
    {
        ESP_LOGI(TAG, "_sendMethodResponse: Failed to get the Methods Response topic");
        return false;
    }

    // Publish the method response.
    int msgId = esp_mqtt_client_publish(
        mqtt_client,
        methods_response_topic_buffer,
        (const char *)az_span_ptr(response),
        az_span_size(response),
        MQTT_QOS1,
        NULL);

    if (msgId == 0)
    {
        ESP_LOGI(TAG, "_sendMethodResponse: Failed to publish the Methods response: MQTTClient return code =0");
        return false;
    }
    else
    {
        ESP_LOGI(TAG, "_sendMethodResponse done, msgId=%d", msgId);
    }
    return true;
}

char payloadBuf[1000];
bool handleCM(esp_mqtt_event_handle_t event)
{
    az_iot_hub_client_method_request request;
    if (az_result_failed(az_iot_hub_client_methods_parse_received_topic(&client, az_span_create((uint8_t *)event->topic, event->topic_len), &request)))
    {
        ESP_LOGI(TAG, "not a method request - %d", event->event_id);
        return false;
    }

    logBuf(TAG, "cm content:", (char *)event->data, event->data_len);

    // if(az_span_is_content_equal(az_span_create_from_str("info"), az_span_create((uint8_t*)event->data, event->data_len))) {
    // c2d_info_handler();
    // }
 

    // TODO remove duplication
    Props props;
    Props::load(props);
    
    az_result result;
    az_span json_buffer = AZ_SPAN_FROM_BUFFER(payloadBuf);
    az_json_writer jw;

    result = az_json_writer_init(&jw, json_buffer, NULL);
    result = az_json_writer_append_begin_object(&jw);    

    result = az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("firmwareVersion"));
    result = az_json_writer_append_string(&jw, az_span_create_from_str(props.getFirmwareVersion()));
    result = az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("gitRevision"));
    result = az_json_writer_append_string(&jw, az_span_create_from_str(props.getGitRevision()));
    result = az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("SSID"));
    result = az_json_writer_append_string(&jw, az_span_create_from_str(WIFI_SSID));
    result = az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("sleep"));
    result = az_json_writer_append_int32(&jw, props.getDoSleep());
    result = az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("batchSize"));
    result = az_json_writer_append_int32(&jw, props.getMeasureBatchSize());
    result = az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("measureInterval"));
    result = az_json_writer_append_int32(&jw, props.getMeasureIntervalMs());
    result = az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("SDA_GPIO"));
    result = az_json_writer_append_int32(&jw, SDA_GPIO);
    result = az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("SCL_GPIO"));
    result = az_json_writer_append_int32(&jw, SCL_GPIO);
    result = az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("ONE_W_PIN"));
    result = az_json_writer_append_int32(&jw, ONE_W_PIN);

    result = az_json_writer_append_end_object(&jw);
    az_span payload = az_json_writer_get_bytes_used_in_destination(&jw);

    _sendMethodResponse(&request, AZ_IOT_STATUS_OK, payload);
    return true;
}

// static bool _is_method(az_span *topic) {
//   return (az_span_find( *topic, methods_topic_request_id)>=0);
// }

// if( _is_method(&topic)) {
//         az_iot_hub_client_methods_parse_received_topic(&client, topic, &methodReq);
//         az_span_to_str((char*)methodNameBuf, sizeof(methodNameBuf), methodReq.name);
//         logMsgStr("\tdirectmethod, msg=", methodNameBuf);
//         az_iot_status statCode = _handleMethod(methodNameBuf, responseBuf, sizeof(responseBuf));
//         logMsgStr("\tdirectmethod, resp=", responseBuf);
//         _sendMethodResponse(&methodReq, statCode, az_span_create_from_str(responseBuf));
//       } else {
// }