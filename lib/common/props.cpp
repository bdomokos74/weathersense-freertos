#include "props.h"
#include "esp_task_wdt.h"

#include "string.h"

#define TAG "PROPS"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"

SemaphoreHandle_t Props::lock;

extern int prop_doSleep;
extern int prop_sleepTimeSec;
extern int prop_measureIntervalMs;
extern int prop_mesaureBatchSize;
extern int prop_ledPin;
extern char *prop_firmwareVersion;
extern char *prop_gitRevision;

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
    if(this->firmwareVersion!=NULL && this->gitRevision!=NULL)
        ESP_LOGI(TAG, "%s{fw=%s, gitrev=%s, sleep=%d, measIntMs=%d, measBatch=%d}", label, this->firmwareVersion, this->gitRevision, 
        this->getDoSleep(), this->measureIntervalMs, this->measureBatchSize);
    else        
        ESP_LOGI(TAG, "%s{sleep=%d, measIntMs=%d, measBatch=%d}", label, 
        this->getDoSleep(), this->measureIntervalMs, this->measureBatchSize);
}
