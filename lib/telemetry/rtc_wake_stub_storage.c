//#include "esp_sleep.h"
//#include "esp_log.h"

int wake_count;

#define RTC_BUF_SIZE 3072

int numStored=0;
int bytesStored=0;
int telemetryId=0;

int prop_doSleep = 0;
int prop_sleepTimeSec = 30;
int prop_measureIntervalMs = 15000;
int prop_mesaureBatchSize = 5;
int prop_ledPin = 0;
//char *prop_firmwareVersion = WS_VERSION;
//char *prop_gitRevision = GIT_REV;
char dataBuf[RTC_BUF_SIZE];