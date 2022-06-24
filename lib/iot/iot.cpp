
// Azure IoT SDK for C includes

#include <az_core.h>
#include <az_iot.h>
#include "iot.h"
#include <Logger.h>
#include "AzIoTSasToken.h"
#include <mqtt_client.h>
#include "ca.h"
#include "telemetry.h"

bool handleTwinResp(esp_mqtt_event_handle_t event) ;
bool handleC2d(esp_mqtt_event_handle_t event);

#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))

extern char* iothubHost;
extern char* iotDeviceId;
extern char* iotDeviceKey;

extern char* mqtt_broker_uri;

// without this api version, the twin api will not work
#define API_VER "/?api-version=2021-04-12"

static const int mqtt_port = AZ_IOT_DEFAULT_MQTT_CONNECT_PORT;

// When developing for your own Arduino-based platform,
// please follow the format '(ard;<platform>)'. 
#define AZURE_SDK_CLIENT_USER_AGENT "c/" AZ_SDK_VERSION_STRING "(ard;esp32)"

// Memory allocated for the sample's variables and structures.
esp_mqtt_client_handle_t mqtt_client;
az_iot_hub_client client;

static char mqtt_client_id[128];
static char mqtt_username[128];
static char mqtt_password[200];
static uint8_t sas_signature_buffer[256];
static char telemetry_topic[128];
static uint8_t telemetry_payload[100];

bool connected = false;

// Auxiliary functions
#ifndef IOT_CONFIG_USE_X509_CERT
static AzIoTSasToken sasToken(
    &client,
    az_span_create_from_str(iotDeviceKey),
    AZ_SPAN_FROM_BUFFER(sas_signature_buffer),
    AZ_SPAN_FROM_BUFFER(mqtt_password));
#endif // IOT_CONFIG_USE_X509_CERT

int cldMsgSubsId;
int twingetSubsId;
int twinPropSubsId;

int cldMsgSubOk = false;
int twingetSubsOk = false;
int twinPropSubsOk = false;
esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
  char buf[1500];
  az_span sp = AZ_SPAN_FROM_BUFFER(buf);
  switch (event->event_id)
  {
    int i, r;
     
    case MQTT_EVENT_ERROR:
      logger.info("MQTT event MQTT_EVENT_ERROR");
    
      az_span_copy_u8(sp, 0);
      logger.info(buf);
      break;
    case MQTT_EVENT_CONNECTED:
      logger.info("MQTT event MQTT_EVENT_CONNECTED");
      

      r = esp_mqtt_client_subscribe(mqtt_client, AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC, 1);
      if (r == -1)
      {
        logger.error("Could not subscribe for cloud-to-device messages.");
      }
      else
      {
        logger.info("Subscribed for cloud-to-device messages; message id:", r);
        cldMsgSubsId = r;
      }

      r = esp_mqtt_client_subscribe(mqtt_client, AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC, 1);
      if (r == -1)
      {
        logger.error("Could not subscribe for twin resp messages.");
      }
      else
      {
        logger.info("Subscribed for twin resp messages; message id:", r);
        twingetSubsId = r;
      }

      r = esp_mqtt_client_subscribe(mqtt_client, AZ_IOT_HUB_CLIENT_PROPERTIES_WRITABLE_UPDATES_SUBSCRIBE_TOPIC, 1);
      if (r == -1)
      {
        logger.error("Could not subscribe for twin prop messages.");
      }
      else
      {
        logger.info("Subscribed for twin prop messages; message id:", r);
        twinPropSubsId = r;
      }

      connected = true;
      break;
    case MQTT_EVENT_DISCONNECTED:
      logger.info("MQTT event MQTT_EVENT_DISCONNECTED");
      connected = false;
      break;
    case MQTT_EVENT_SUBSCRIBED:
      logger.info("MQTT event MQTT_EVENT_SUBSCRIBED msgid=", event->msg_id);
      if(event->msg_id==cldMsgSubsId) {
        cldMsgSubOk = true;
      } else if(event->msg_id==twingetSubsId) {
        twingetSubsOk = true;
      } else if(event->msg_id==twinPropSubsId) {
        twinPropSubsOk = true;
      }
      break;
    case MQTT_EVENT_UNSUBSCRIBED:
      logger.info("MQTT event MQTT_EVENT_UNSUBSCRIBED");
      break;
    case MQTT_EVENT_PUBLISHED:
      logger.info("MQTT event MQTT_EVENT_PUBLISHED");
      break;
    case MQTT_EVENT_DATA:
      logger.info("MQTT event MQTT_EVENT_DATA");
      logger.printBuf("topic: ", event->topic, event->topic_len);
      
      if(handleTwinResp(event)) {
        logger.info("twin handled");
      } else if(handleC2d(event) ) {
        logger.info("c2d handled");
      } else {
        logger.info("msg not handled");
      }

      break;
    case MQTT_EVENT_BEFORE_CONNECT:
      logger.info("MQTT event MQTT_EVENT_BEFORE_CONNECT");
      break;
    default:
      logger.error("MQTT event UNKNOWN");
      break;
  }

  return ESP_OK;
}

void initializeIoTHubClient()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.user_agent = AZ_SPAN_FROM_STR(AZURE_SDK_CLIENT_USER_AGENT);

  if (az_result_failed(az_iot_hub_client_init(
          &client,
          az_span_create((uint8_t*)iothubHost, strlen(iothubHost)),
          az_span_create((uint8_t*)iotDeviceId, strlen(iotDeviceId)),
          &options)))
  {
    logger.error("Failed initializing Azure IoT Hub client");
    return;
  }

  size_t client_id_length;
  if (az_result_failed(az_iot_hub_client_get_client_id(
          &client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length)))
  {
    logger.error("Failed getting client id");
    return;
  }

  az_span username_span = AZ_SPAN_FROM_BUFFER(mqtt_username);
  az_span remainder = az_span_copy(username_span, az_span_create_from_str(iothubHost));
  remainder = az_span_copy_u8(remainder, '/');
  remainder = az_span_copy(remainder, az_span_create_from_str(iotDeviceId));
  remainder = az_span_copy(remainder, AZ_SPAN_LITERAL_FROM_STR(API_VER));
  remainder = az_span_copy_u8(remainder, 0);

  // if (az_result_failed(az_iot_hub_client_get_user_name(
  //         &client, mqtt_username, sizeofarray(mqtt_username), NULL)))
  // {
  //   logger.error("Failed to get MQTT clientId, return code");
  //   return;
  // }

  logger.println("Client ID: ", mqtt_client_id);
  logger.println("Username: ", mqtt_username);
  logger.println("broker uri: ", mqtt_broker_uri);
}

int initializeMqttClient()
{
  #ifndef IOT_CONFIG_USE_X509_CERT
  if (sasToken.Generate(SAS_TOKEN_DURATION_IN_MINUTES) != 0)
  {
    logger.error("Failed generating SAS token");
    return 1;
  }
  #endif

  esp_mqtt_client_config_t mqtt_config;
  memset(&mqtt_config, 0, sizeof(mqtt_config));
  mqtt_config.uri = mqtt_broker_uri;
  mqtt_config.port = mqtt_port;
  mqtt_config.client_id = mqtt_client_id;
  mqtt_config.username = mqtt_username;
  //mqtt_config.protocol_ver = MQTT_PROTOCOL_V_3_1_1;

  #ifdef IOT_CONFIG_USE_X509_CERT
    logger.info("MQTT client using X509 Certificate authentication");
    mqtt_config.client_cert_pem = IOT_CONFIG_DEVICE_CERT;
    mqtt_config.client_key_pem = IOT_CONFIG_DEVICE_CERT_PRIVATE_KEY;
  #else // Using SAS key
    mqtt_config.password = (const char*)az_span_ptr(sasToken.Get());
  #endif

  mqtt_config.keepalive = 30;
  mqtt_config.disable_clean_session = 0;
  mqtt_config.disable_auto_reconnect = false;
  mqtt_config.event_handle = mqtt_event_handler;
  mqtt_config.user_context = NULL;
  mqtt_config.cert_pem = (const char*)ca_pem;
  //mqtt_config.cert_len = caw_pem_len;
  mqtt_client = esp_mqtt_client_init(&mqtt_config);

  if (mqtt_client == NULL)
  {
    logger.error("Failed creating mqtt client");
    return 1;
  }

  esp_err_t start_result = esp_mqtt_client_start(mqtt_client);

  if (start_result != ESP_OK)
  {
    logger.error("Could not start mqtt client; error code");
    return 1;
  }
  else
  {
    logger.info("MQTT client started");
    return 0;
  }
}

int sendTelemetry()
{
  logger.info("Sending telemetry ...");

  if(!connected||!cldMsgSubOk) {
    logger.println("not connected, skip telemetry");
    return 0;
  }
  az_span telemetry = AZ_SPAN_FROM_BUFFER(telemetry_payload);

  // The topic could be obtained just once during setup,
  // however if properties are used the topic need to be generated again to reflect the
  // current values of the properties.
  size_t topic_len;
  if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(
          &client, NULL, telemetry_topic, sizeof(telemetry_topic), &topic_len)))
  {
    logger.error("Failed az_iot_hub_client_telemetry_get_publish_topic");
    return 0;
  }
  telemetry_topic[topic_len] = 0;
  logger.println("topic: ", telemetry_topic);


  getTelemetryPayload(telemetry, &telemetry);

  int ret = esp_mqtt_client_publish(
          mqtt_client,
          telemetry_topic,
          (const char*)az_span_ptr(telemetry),
          az_span_size(telemetry),
          0,
          DO_NOT_RETAIN_MSG);
  if (ret == -1)
  {
    logger.error("Failed publishing");
    return 0;
  }
  else
  {
    logger.info("Message published successfully: ", ret);
    return 1;
  }
}

