#ifndef _TELEMETRY_H
#define _TELEMETRY_H

#include <az_core.h>

class Telemetry {
protected:
    char *dataBuf;
    int bufLen;
    char *bufPoi;
    int *numStored;
    int *telemetryId;
public:
    Telemetry(
        char *dataBuf,
        int bufLen,
        char *poi,
        int *numStored,
        int *telemetryId);

    void buildTelemetryPayload(az_span payload, az_span* out_payload, float t, float p, float h);

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
