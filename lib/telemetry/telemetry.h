#ifndef _TELEMETRY_H
#define _TELEMETRY_H

#include <az_core.h>

class Telemetry {
private:
    char *dataBuf;
    int bufLen;
    char **bufPoi;
    int *numStored;
public:
    Telemetry(
        char *dataBuf,
        int bufLen,
        char **poi,
        int *numStored);

    void buildTelemetryPayload(az_span payload, az_span* out_payload, float t, float p, float h);

    int getStoredSize();
    int getRemainingSize();
    bool doesMeasurementFit(az_span meas);
    int storeMeasurement(az_span meas);
    
    void buildStatus(char *buf, int len);
    void reset();
};

#endif
