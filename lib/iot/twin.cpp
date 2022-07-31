#include "iot.h"
#include <az_core.h>
#include <az_iot.h>
#include <Logger.h>
#include <mqtt_client.h>
#include "props.h"
#include "tsafevars.h"

#include "esp_log.h"
#define TAG "TWIN"

void ws_start_ota(char * str);

extern az_iot_hub_client client;
extern esp_mqtt_client_handle_t mqtt_client;
//extern bool connected;

static char twin_req_topic[128];
static char twin_patch_topic[100];

az_span topicSpan;
az_span printSpan;
az_span dataSpan;
az_span eventDataSpan;

#define INCOMING_DATA_BUFFER_SIZE 1024
static char incoming_data[INCOMING_DATA_BUFFER_SIZE];

static char desiredVersion[20];

az_span measureIntervalMsToken = AZ_SPAN_LITERAL_FROM_STR("measureIntervalMs");
az_span measureBatchSizeToken = AZ_SPAN_LITERAL_FROM_STR("measureBatchSize");
az_span sleepTimeSecToken = AZ_SPAN_LITERAL_FROM_STR("sleepTimeSec");
az_span doSleepToken = AZ_SPAN_LITERAL_FROM_STR("doSleep");
az_span fwVersionToken = AZ_SPAN_LITERAL_FROM_STR("fwVersion");
az_span gitRevisionToken = AZ_SPAN_LITERAL_FROM_STR("gitRevision");

void printTwinResponseDetails(az_iot_hub_client_twin_response response);

extern TSafeVars mainTSafeVars;

int twinGetReqId = 1;
static char reqIdBuf[16];
// https://docs.microsoft.com/en-us/azure/iot-hub/troubleshoot-error-codes#404104-deviceconnectionclosedremotely
bool requestTwin()
{
    ESP_LOGI(TAG, "Requesting twin get...");

    az_result result;
    az_span reqIdSpan = AZ_SPAN_FROM_STR("137");

    size_t s;
    if (az_result_failed(az_iot_hub_client_twin_document_get_publish_topic(
            &client, reqIdSpan, twin_req_topic, sizeof(twin_req_topic), &s)))
    {
        ESP_LOGE(TAG, "Failed az_iot_hub_client_properties_document_get_publish_topic");
        return false;
    }
    twin_req_topic[s] = 0;
    ESP_LOGI(TAG, "twintopic: %s", twin_req_topic);
    
    int ret = esp_mqtt_client_publish(
        mqtt_client,
        twin_req_topic,
        NULL,
        0,
        0,
        false);
    if (ret == -1)
    {
        ESP_LOGE(TAG, "Failed publishing twin req");
        return false;
    }
    else
    {
        ESP_LOGI(TAG, "Twin req published successfully");
        return true;
    }
}

static char tokenBuf[30];
Props* parseTwinResp(az_span twinResp)
{
    az_result result;
    desiredVersion[0] = 0;
    int measureBatchSize;
    int measureIntervalMs;
    int sleepTimeSec;
    int doSleep;

    bool propertyFound = false;
    az_json_reader jr;
    result = az_json_reader_init(&jr, twinResp, NULL);
    result = az_json_reader_next_token(&jr);
    if (jr.token.kind != AZ_JSON_TOKEN_BEGIN_OBJECT)
    {
        ESP_LOGE(TAG, "invalid twin resp");
    }

    result = az_json_reader_next_token(&jr);
    
    while (!propertyFound && (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT))
    {
        az_span tokenSpan = AZ_SPAN_FROM_BUFFER(tokenBuf);
        az_span reminder = az_json_token_copy_into_span(&jr.token, tokenSpan);
        az_span_copy_u8(reminder, 0);
        //ESP_LOGI(TAG, "TOKEN: %s", tokenBuf);
        if (az_json_token_is_text_equal(&jr.token, fwVersionToken))
        {
            result = az_json_reader_next_token(&jr);
            int versionLen;
            result = az_json_token_get_string(&jr.token, desiredVersion, sizeof(desiredVersion), &versionLen);
            desiredVersion[versionLen] = 0;
        }
        else if (az_json_token_is_text_equal(&jr.token, measureBatchSizeToken))
        {
            result = az_json_reader_next_token(&jr);
            result = az_json_token_get_int32(&jr.token, &measureBatchSize);
        }
        else if (az_json_token_is_text_equal(&jr.token, measureIntervalMsToken))
        {
            result = az_json_reader_next_token(&jr);
            result = az_json_token_get_int32(&jr.token, &measureIntervalMs);
        }
        else if (az_json_token_is_text_equal(&jr.token, doSleepToken))
        {
            result = az_json_reader_next_token(&jr);
            result = az_json_token_get_int32(&jr.token, &doSleep);
        }
        else if (az_json_token_is_text_equal(&jr.token, sleepTimeSecToken))
        {
            result = az_json_reader_next_token(&jr);
            result = az_json_token_get_int32(&jr.token, &sleepTimeSec);
        } else {
            // ignore
            result = az_json_reader_next_token(&jr);
        }
        result = az_json_reader_next_token(&jr);
    }
    
    Props *desired = new Props(desiredVersion, doSleep, sleepTimeSec, measureIntervalMs, measureBatchSize, -1);
    ESP_LOGI(TAG, "====parse twin resp end====");
    desired->debug("desired properties");
    return desired;
}

Props currProps;
bool handleTwinResp(esp_mqtt_event_handle_t event)
{
    az_iot_hub_client_twin_response response;
    if (az_result_failed(az_iot_hub_client_twin_parse_received_topic(&client, az_span_create((uint8_t *)event->topic, event->topic_len), &response)))
    {
        ESP_LOGI(TAG, "not twin");
        return false;
    }

    printTwinResponseDetails(response);

    if (response.response_type == AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_GET)
    {
        logBuf(TAG, "RESP data: ", event->data, event->data_len);

        Props *resp = parseTwinResp(az_span_create((uint8_t *)event->data, event->data_len));
        resp->debug("twinresp");
        Props::merge(*resp);
        Props::load(*resp);
        mainTSafeVars.setTwinGetSuccess();
        delete resp;

        return true;
    } else if( response.response_type == AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_DESIRED_PROPERTIES) {
        logBuf(TAG, "DESIRED, req data: ", event->data, event->data_len);
        
        Props *desired = parseTwinResp(az_span_create((uint8_t *)event->data, event->data_len));
        Props::load(currProps);

        if(strcmp(desired->getFirmwareVersion(), currProps.getFirmwareVersion())!=0) {
            ESP_LOGI(TAG, "calling ws_start_ota %s <> %s", desired->getFirmwareVersion(), currProps.getFirmwareVersion());
            ws_start_ota(NULL);
        }

        currProps.debug("curr");
        //desired->debug("desired");        
        Props::merge(*desired);
        delete desired;

        //mainTSafeVars.setTwinPatchSuccess();
        sendTwinProp();
        //logger.printBuf("DESIRED: ", event->data, event->data_len);
        return true;
    } else if( response.response_type == AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_REPORTED_PROPERTIES) {
        //twin: resptype=3 reqid=ptch_tmp st=204 ver=11958
        //2022/6/5 23:11:58 c2d topic: $iothub/twin/res/204/?$rid=ptch_tmp&$version=11958
        logBuf(TAG, "REPORTED: ", event->data, event->data_len);
        mainTSafeVars.setTwinPatchSuccess();
        return true;
    }

    return false;
}

static char printBuf[200];
void printTwinResponseDetails(az_iot_hub_client_twin_response response)
{
    az_result result;
    az_span printSpan = AZ_SPAN_FROM_BUFFER(printBuf);
    az_span rem = az_span_copy(printSpan, AZ_SPAN_LITERAL_FROM_STR("twin: resptype="));
    result = az_span_u32toa(rem, (int)response.response_type, &rem);
    rem = az_span_copy(rem, AZ_SPAN_LITERAL_FROM_STR(" reqid="));
    rem = az_span_copy(rem, response.request_id);
    rem = az_span_copy(rem, AZ_SPAN_LITERAL_FROM_STR(" st="));
    result = az_span_u32toa(rem, (int)response.status, &rem);
    rem = az_span_copy(rem, AZ_SPAN_LITERAL_FROM_STR(" ver="));
    rem = az_span_copy(rem, response.version);
    az_span_copy_u8(rem, 0);
    ESP_LOGI(TAG, "%s", printBuf);
}

static char twinSendBuf[1024];
void sendTwinProp()
{
    ESP_LOGI(TAG, "sending twin patch");
    
    Props props;
    Props::load(props);
    
    az_result result;
    az_span json_buffer = AZ_SPAN_FROM_BUFFER(twinSendBuf);
    az_json_writer jw;

    result = az_json_writer_init(&jw, json_buffer, NULL);
    result = az_json_writer_append_begin_object(&jw);    

    result = az_json_writer_append_property_name(&jw, fwVersionToken);
    result = az_json_writer_append_string(&jw, az_span_create_from_str(props.getFirmwareVersion()));
    result = az_json_writer_append_property_name(&jw, gitRevisionToken);
    result = az_json_writer_append_string(&jw, az_span_create_from_str(props.getGitRevision()));
    result = az_json_writer_append_property_name(&jw, doSleepToken);
    result = az_json_writer_append_int32(&jw, props.getDoSleep());
    result = az_json_writer_append_property_name(&jw, measureBatchSizeToken);
    result = az_json_writer_append_int32(&jw, props.getMeasureBatchSize());
    result = az_json_writer_append_property_name(&jw, measureIntervalMsToken);
    result = az_json_writer_append_int32(&jw, props.getMeasureIntervalMs());
    result = az_json_writer_append_property_name(&jw, sleepTimeSecToken);
    result = az_json_writer_append_int32(&jw, props.getSleepTimeSec());

    result = az_json_writer_append_end_object(&jw);
    az_span twin_payload = az_json_writer_get_bytes_used_in_destination(&jw);

    logSpan(TAG, "patch payload=", twin_payload);

    //"$iothub/twin/PATCH/properties/reported/?$rid=patch_temp";

    // The topic could be obtained just once during setup,
    // however if properties are used the topic need to be generated again to reflect the
    // current values of the properties.

    size_t topic_len;
    if (az_result_failed(az_iot_hub_client_twin_patch_get_publish_topic(
            &client, AZ_SPAN_LITERAL_FROM_STR("ptch_tmp"), twin_patch_topic, sizeof(twin_patch_topic), &topic_len)))
    {
        ESP_LOGE(TAG, "Failed az_iot_hub_client_telemetry_get_publish_topic");
        return;
    }

    logBuf(TAG, "twin patch topic: ", twin_patch_topic, topic_len);

    if (esp_mqtt_client_publish(
            mqtt_client,
            twin_patch_topic,
            (char*)az_span_ptr(twin_payload),
            az_span_size(twin_payload),
            0,
            DO_NOT_RETAIN_MSG) == -1)
    {
        ESP_LOGE(TAG, "Failed publishing");
    }
    else
    {
        ESP_LOGI(TAG, "Message published successfully");
    }
}
