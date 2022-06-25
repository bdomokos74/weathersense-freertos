
#include <az_core.h>

void telemetryInit(int sda, int scl);
void getTelemetryPayload(az_span payload, az_span* out_payload, float t, float p, float h);