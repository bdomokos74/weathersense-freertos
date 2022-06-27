#ifndef _WS_TSAFEVARS_H
#define _WS_TSAFEVARS_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class TSafeVars
{
private:
    SemaphoreHandle_t lock;
    bool mqttRestart;
public:
    TSafeVars();

    bool getAndClearMqttRestartReqested();
    void reqestMqttRestart();
    
};

#endif