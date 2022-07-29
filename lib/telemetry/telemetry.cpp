#include "telemetry.h"

//#include "esp_log.h"

#include <time.h>
#include "stdio.h"
#include "wscommon.h"

#define TAG "TLMTRY"

static int sensor_count = 0;


Telemetry::Telemetry(
    char *dataBuf,
    int *bytesStored,
    int *numStored,
    int *tmId) {
        this->dataBuf = dataBuf;
        this->bytesStored = bytesStored;
        this->numStored = numStored;
        this->telemetryId = tmId;
}


void Telemetry::buildTelemetryPayload(az_span payload, az_span* out_payload, float temperature, float temperature2, float pressure, float humidity)
{
    buildTelemetryPayload(payload, out_payload,
    true, temperature, 
    true, temperature2,
    true, pressure,
    true, humidity);
}

void Telemetry::buildTelemetryPayload(az_span payload, az_span* out_payload, float temperature, float pressure, float humidity)
{
    buildTelemetryPayload(payload, out_payload,
    true, temperature, 
    false, 0,
    true, pressure,
    true, humidity);
}

void Telemetry::buildTelemetryPayload(az_span payload, az_span* out_payload, 
bool showt1, float temperature, 
bool showt2, float temperature2, 
bool showp, float pressure, 
bool showh, float humidity)
{
  az_span tempSpan;
  az_span tempSpan2;
  az_span pSpan;
  az_span hSpan;

  char tempBuf[20];

  // --------
  az_span original_payload = payload;

  payload = az_span_copy(payload, AZ_SPAN_FROM_STR("{\"id\":"));
  (void)az_span_u32toa(payload, (*(this->telemetryId))++, &payload);

  payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"ts\":"));
  time_t now = time(NULL);
  (void)az_span_u64toa(payload, now, &payload);
  
  if(showt1) {
    payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"t1\":"));
    sprintf(tempBuf, "%.2f", temperature);
    tempSpan = az_span_create_from_str(tempBuf);
    payload = az_span_copy(payload, tempSpan);
  }

  if(showt2) {
    payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"t2\":"));
    sprintf(tempBuf, "%.2f", temperature2);
    tempSpan2 = az_span_create_from_str(tempBuf);
    payload = az_span_copy(payload, tempSpan2);
  }

  if(showp) {
    payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"p\":"));
    sprintf(tempBuf, "%.2f", pressure);
    pSpan = az_span_create_from_str(tempBuf);
    payload = az_span_copy(payload, pSpan);
  }

  if(showh) {
    payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"h\":"));
    sprintf(tempBuf, "%.2f", humidity);
    hSpan = az_span_create_from_str(tempBuf);
    payload = az_span_copy(payload, hSpan);
  }
  payload = az_span_copy(payload, AZ_SPAN_FROM_STR("}"));
  payload = az_span_copy_u8(payload, '\0');
  
  *out_payload = az_span_slice(original_payload, 0, az_span_size(original_payload) - az_span_size(payload) - 1);
}

char *Telemetry::getDataBuf() {
    return this->dataBuf;
}


int Telemetry::getNumStored() {
    return *(this->numStored);
}

int Telemetry::getRemainingSize() {
    return RTC_BUF_SIZE-(*this->bytesStored);
}

int Telemetry::getStoredSize() {
    return *(this->bytesStored);
}

bool Telemetry::doesMeasurementFit(az_span meas) {
    return (az_span_size(meas)+1)<=this->getRemainingSize();
}

int Telemetry::storeMeasurement(az_span meas) {
    //ESP_LOGI("TELEMETRY", "storing meas, databuf=%p, bytesStored=%d, numStored=%d, remainingBytes=%d", this->dataBuf, *this->bytesStored, *this->numStored, this->getRemainingSize());
    az_span buf = az_span_create((uint8_t*)(this->dataBuf+*(this->bytesStored)), this->getRemainingSize());
    buf = az_span_copy(buf, meas);
    buf = az_span_copy_u8(buf, '\n');
    *(this->bytesStored) = ((char*)az_span_ptr(buf) - this->dataBuf);
    (*this->numStored)++;
    //ESP_LOGI("TELEMETRY AFTER", "storing meas, databuf=%p, bytesStored=%d, numStored=%d, remainingBytes=%d", this->dataBuf, *this->bytesStored, *this->numStored, this->getRemainingSize());
    //hexDump("buffer", this->numStored, 160, 16);
    return 1;
}

void Telemetry::buildStatus(char *buf, int len) {
    snprintf(buf, len, "MEAS_STORE: numStored=%d, storedBytes=%d, remainingBytes=%d", this->getNumStored(), this->getStoredSize(), this->getRemainingSize());
}

void Telemetry::reset() {
    *(this->numStored) = 0;
    *(this->bytesStored) = 0;
}
