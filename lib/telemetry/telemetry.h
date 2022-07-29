#ifndef _TELEMETRY_H
#define _TELEMETRY_H

#include <az_core.h>

// TODO duplicated, also defined in rtc_wake_stub_storage.c
#define RTC_BUF_SIZE 3072

class Telemetry {
protected:
    char *dataBuf;
    int *bytesStored;
    int *numStored;
    int *telemetryId;
public:
    Telemetry(
        char *dataBuf,
        int *bytesStored,
        int *numStored,
        int *telemetryId);

    void buildTelemetryPayload(az_span payload, az_span* out_payload, float t, float p, float h);
    void buildTelemetryPayload(az_span payload, az_span* out_payload, float t, float t2, float p, float h);
    void buildTelemetryPayload(az_span payload, az_span* out_payload, 
        bool b1, float t, 
        bool b2, float t2, 
        bool b3, float p, 
        bool b4, float h);

    char *getDataBuf();
    int getNumStored();
    int getStoredSize();
    int getRemainingSize();
    bool doesMeasurementFit(az_span meas);
    int storeMeasurement(az_span meas);
    
    void buildStatus(char *buf, int len);
    void reset();
};

#endif
