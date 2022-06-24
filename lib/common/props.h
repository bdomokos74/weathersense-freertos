#ifndef _WS_PROPS_H
#define _WS_PROPS_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class Props
{
private:
    char *firmwareVersion;
    char *gitRevision;
    int doSleep;
    int sleepTimeSec;
    int measureIntervalMs;
    int measureBatchSize;
    int ledPin;
    static SemaphoreHandle_t lock;
public:
    Props();
    Props(
        char *firmwareVersion,
        int doSleep,
        int sleepTimeSec,
        int measureIntervalMs,
        int measureBatchSize,
        int ledPin
     );
    static void init();
    
    static void merge(Props &p);
    static void load(Props &p);

    char* getFirmwareVersion();
    char* getGitRevision();
    int getDoSleep();
    int getSleepTimeSec();
    int getMeasureIntervalMs();
    int getMeasureBatchSize();
    int getLedPin();

    void debug(char*);
};

#endif