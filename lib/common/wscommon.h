#ifndef _WSCOMMON_H
#define _WSCOMMON_H

#define WSOK 1
#define WSNOK 0

#define uS_TO_S_FACTOR 1000000
#define mS_TO_S_FACTOR 1000

// TODO initialize properly
#define SDA_GPIO 21
#define SCL_GPIO 22
#define WIFI_FAIL_SLEEP_TIME_SEC 60*30

#define ONE_W_PIN 19
// 21

void hexDump (
    const char * desc,
    const void * addr,
    const int len,
    int perLine
);

#endif
