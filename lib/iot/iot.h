#ifndef _WIOT_H
#define _WIOT_H

#include "az_core.h"

#define MQTT_QOS1 1
#define DO_NOT_RETAIN_MSG 0

// TODO make this a parameter
#define SAS_TOKEN_DURATION_IN_MINUTES 60

void initializeIoTHubClient();
int initializeMqttClient();
int sendTelemetry(az_span telemetry);
bool requestTwin();
void mqttClientDisconnect();
void mqttClientDestroy();

#endif
