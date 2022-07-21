#ifndef _WS_TSAFEVARS_H
#define _WS_TSAFEVARS_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class TSafeVars
{
private:
    SemaphoreHandle_t lock;
    bool mqttRestart;
    bool twinGetSuccess;
    bool twinPatchSuccess;
    bool disconnected;
public:
    TSafeVars();

    bool getAndClearMqttRestartReqested();
    void reqestMqttRestart();
    
    bool getAndClearTwinGetSuccess();
    void setTwinGetSuccess();

    bool getAndClearTwinPatchSuccess();
    void setTwinPatchSuccess();

    bool getAndClearDisconnected();
    void setDisconnected();
};

#endif