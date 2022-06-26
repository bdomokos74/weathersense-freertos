#include "props.h"
#include "esp_task_wdt.h"

#include "string.h"

#define TAG "PROPS"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"

SemaphoreHandle_t Props::lock;

RTC_DATA_ATTR int prop_doSleep = 0;
RTC_DATA_ATTR int prop_sleepTimeSec = 30;
RTC_DATA_ATTR int prop_measureIntervalMs = 30000;
RTC_DATA_ATTR int prop_mesaureBatchSize = 5;
RTC_DATA_ATTR int prop_ledPin = 0;
RTC_DATA_ATTR char *prop_firmwareVersion = WS_VERSION;
RTC_DATA_ATTR char *prop_gitRevision = GIT_REV;

void Props::init() {
    BaseType_t rc;
    Props::lock = xSemaphoreCreateBinary();
    rc = xSemaphoreGive(Props::lock);
}

Props::Props() {
    this->firmwareVersion = nullptr;
    this->gitRevision = nullptr;
    this->doSleep = -1;
    this->sleepTimeSec = -1;
    this->measureIntervalMs = -1;
    this->measureBatchSize = -1;
    this->ledPin = -1;
}

Props::Props(
    char *firmwareVersion,
    int doSleep,
    int sleepTimeSec,
    int measureIntervalMs,
    int measureBatchSize,
    int ledPin
) {
    this->firmwareVersion = firmwareVersion;
    this->gitRevision = nullptr;
    this->doSleep = doSleep;
    this->sleepTimeSec = sleepTimeSec;
    this->measureIntervalMs = measureIntervalMs;
    this->measureBatchSize = measureBatchSize;
    this->ledPin =ledPin;
}

void Props::load(Props &props) {
    xSemaphoreTake(Props::lock, portMAX_DELAY);
    props.doSleep = prop_doSleep;
    props.sleepTimeSec = prop_sleepTimeSec;
    props.measureIntervalMs = prop_measureIntervalMs;
    props.measureBatchSize = prop_mesaureBatchSize;
    props.ledPin = prop_ledPin;
    props.firmwareVersion = prop_firmwareVersion;
    props.gitRevision = prop_gitRevision;
    xSemaphoreGive(Props::lock);
}

void Props::merge(Props &props) {
    xSemaphoreTake(Props::lock, portMAX_DELAY);
    if(props.doSleep!=-1) prop_doSleep = props.doSleep;
    if(props.sleepTimeSec!=-1) prop_sleepTimeSec = props.sleepTimeSec;
    if(props.measureIntervalMs!=-1) prop_measureIntervalMs = props.measureIntervalMs;
    if(props.measureBatchSize!=-1) prop_mesaureBatchSize = props.measureBatchSize;
    if(props.ledPin!=-1) prop_ledPin = props.ledPin;
    xSemaphoreGive(Props::lock);
}

char* Props::getFirmwareVersion() {
    return this->firmwareVersion;
}

char* Props::getGitRevision() {
    return this->gitRevision;
}

int Props::getDoSleep(){
    return doSleep;
}

int Props::getSleepTimeSec(){
    return sleepTimeSec;
}

int Props::getMeasureIntervalMs(){
    return measureIntervalMs;
}

int Props::getMeasureBatchSize(){
    return measureBatchSize;
}

int Props::getLedPin(){
    return ledPin;
}

void Props::debug(char *label) {
    ESP_LOGI(TAG, "props.%s{fw=%s, gitrev=%s, measIntMs=%d, measBatch=%d}", label, this->firmwareVersion, this->gitRevision, this->measureIntervalMs, 
    this->measureBatchSize);
}
