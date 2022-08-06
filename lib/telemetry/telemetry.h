#ifndef _TELEMETRY_H
#define _TELEMETRY_H

#include <az_core.h>
#include "basesensor.h"

#define MAX_SENSOR 4

class Telemetry {
protected:
    char *dataBuf;
    int bufSize;
    int *bytesStored;
    int *numStored;
    int *telemetryId;

    BaseSensor *sensors[MAX_SENSOR];
    int numSensors;

public:
    Telemetry(
        char *dataBuf,
        int bufSize,
        int *bytesStored,
        int *numStored,
        int *telemetryId);

    bool buildTelemetryPayload(az_span payload, az_span* out_payload);

    char *getDataBuf();
    int getNumStored();
    int getStoredSize();
    int getRemainingSize();
    bool doesMeasurementFit(az_span meas);
    int storeMeasurement(az_span meas);
    
    void buildStatus(char *buf, int len);
    void reset();

    void addSensor(BaseSensor *s);
};

#endif
