#include "telemetry.h"

//#include "esp_log.h"
#include <time.h>
#include "stdio.h"
#include "wscommon.h"

#define TAG "TLMTRY"

static int sensor_count = 0;

static float temperature;
static float pressure;
static float humidity;

static uint32_t telemetry_send_count = 0;

Telemetry::Telemetry(
    char *dataBuf,
    int bufLen,
    char **bufPoi,
    int *numStored) {
        this->dataBuf = dataBuf;
        this->bufLen = bufLen;
        this->bufPoi = bufPoi;
        this->numStored = numStored;
        *(this->bufPoi) = dataBuf;
}

static char tempBuf[20];
void Telemetry::buildTelemetryPayload(az_span payload, az_span* out_payload, float temperature, float pressure, float humidity)
{
  sprintf(tempBuf, "%.2f", temperature);
  az_span tempSpan = az_span_create_from_str(tempBuf);

  char pBuf[20];
  sprintf(pBuf, "%.2f", pressure);
  az_span pSpan = az_span_create_from_str(pBuf);

  char hBuf[20];
  sprintf(hBuf, "%.2f", humidity);
  az_span hSpan = az_span_create_from_str(hBuf);

  // --------
  az_span original_payload = payload;

  payload = az_span_copy(payload, AZ_SPAN_FROM_STR("{\"id\":"));
  (void)az_span_u32toa(payload, telemetry_send_count++, &payload);

  payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"ts\":"));
  time_t now = time(NULL);
  (void)az_span_u64toa(payload, now, &payload);
  
  payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"t1\":"));
  payload = az_span_copy(payload, tempSpan);
  payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"p\":"));
  payload = az_span_copy(payload, pSpan);
  payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"h\":"));
  payload = az_span_copy(payload, hSpan);

  payload = az_span_copy(payload, AZ_SPAN_FROM_STR("}"));
  payload = az_span_copy_u8(payload, '\0');
  
  *out_payload = az_span_slice(original_payload, 0, az_span_size(original_payload) - az_span_size(payload) - 1);
}

int Telemetry::getRemainingSize() {
    return this->bufLen-(int)((*(this->bufPoi))-this->dataBuf);
}

int Telemetry::getStoredSize() {
    return (int)((*(this->bufPoi))-this->dataBuf);
}

bool Telemetry::doesMeasurementFit(az_span meas) {
    return (az_span_size(meas)+1)<=this->getRemainingSize();
}

int Telemetry::storeMeasurement(az_span meas) {
    az_span buf = az_span_create((uint8_t*)*this->bufPoi, this->getRemainingSize());
    buf = az_span_copy(buf, meas);
    buf = az_span_copy_u8(buf, '\n');
    *(this->bufPoi) = (char*)az_span_ptr(buf);
    (*this->numStored)++;
    return 1;
}

void Telemetry::buildStatus(char *buf, int len) {
    snprintf(buf, len, "MEAS_STORE: numStored=%d, storedBytes=%d, remainingBytes=%d", *numStored, this->getStoredSize(), this->getRemainingSize());
}

void Telemetry::reset() {
    *this->bufPoi = this->dataBuf;
    (*numStored) = 0;
}
