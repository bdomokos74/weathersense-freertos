#include "tsafevars.h"

TSafeVars::TSafeVars() {
    BaseType_t rc;
    this->lock = xSemaphoreCreateBinary();
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
