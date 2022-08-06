#include "tsafevars.h"

TSafeVars::TSafeVars() {
    BaseType_t rc;
    this->lock = xSemaphoreCreateBinary();
    rc = xSemaphoreGive(this->lock);
}

bool TSafeVars::getAndClearTwinGetSubscribed() {
    BaseType_t rc;
    bool result;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    result = this->twinGetSubscribed;
    if(result==true) {
        this->twinGetSubscribed = false;
    }
    rc = xSemaphoreGive(this->lock);
    return result;
}

void TSafeVars::setTwinGetSubscribed() {
    BaseType_t rc;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    this->twinGetSubscribed = true;
    rc = xSemaphoreGive(this->lock);
}

bool TSafeVars::getAndClearMqttRestartReqested() {
    BaseType_t rc;
    bool result;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    result = this->mqttRestart;
    if(result==true) {
        this->mqttRestart = false;
    }
    rc = xSemaphoreGive(this->lock);
    return result;
}

void TSafeVars::reqestMqttRestart() {
    BaseType_t rc;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    this->mqttRestart = true;
    rc = xSemaphoreGive(this->lock);
}

bool TSafeVars::getAndClearTwinGetSuccess() {
    BaseType_t rc;
    bool result;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    result = this->twinGetSuccess;
    if(result==true) {
        this->twinGetSuccess = false;
    }
    rc = xSemaphoreGive(this->lock);
    return result;

}
void TSafeVars::setTwinGetSuccess() {
    BaseType_t rc;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    this->twinGetSuccess = true;
    rc = xSemaphoreGive(this->lock);
}

bool TSafeVars::getAndClearTwinPatchSuccess() {
    BaseType_t rc;
    bool result;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    result = this->twinPatchSuccess;
    if(result==true) {
        this->twinPatchSuccess = false;
    }
    rc = xSemaphoreGive(this->lock);
    return result;
}

void TSafeVars::setTwinPatchSuccess() {
    BaseType_t rc;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    this->twinPatchSuccess = true;
    rc = xSemaphoreGive(this->lock);
}


bool TSafeVars::getAndClearDisconnected() {
    BaseType_t rc;
    bool result;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    result = this->disconnected;
    if(result==true) {
        this->disconnected = false;
    }
    rc = xSemaphoreGive(this->lock);
    return result;
}

void TSafeVars::setDisconnected() {
    BaseType_t rc;
    rc = xSemaphoreTake(this->lock, portMAX_DELAY);
    this->disconnected = true;
    rc = xSemaphoreGive(this->lock);
}