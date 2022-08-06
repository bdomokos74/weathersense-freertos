#include "telemetry.h"

//#include "esp_log.h"

#include <time.h>
#include "stdio.h"
#include "wscommon.h"

#define TAG "TLMTRY"

static int sensor_count = 0;


Telemetry::Telemetry(
    char *dataBuf,
    int bufSize,
    int *bytesStored,
    int *numStored,
    int *tmId) {
        this->dataBuf = dataBuf;
        this->bufSize = bufSize;
        this->bytesStored = bytesStored;
        this->numStored = numStored;
        this->telemetryId = tmId;
        this->numSensors = 0;
}

bool Telemetry::buildTelemetryPayload(az_span payload, az_span* out_payload)
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

  int tIdx = 1;
  bool found = false;
  for(int i = 0; i < this->numSensors; i++) {
    BaseSensor *s = this->sensors[i];
    float f1; bool b1;
    float f2; bool b2;
    float f3; bool b3;
    s->readMeasurement(f1, b1, f2, b2, f3, b3);
    if(b1) {
        payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"t"));
        az_span_i32toa(payload, tIdx++, &payload);
        payload = az_span_copy(payload, AZ_SPAN_FROM_STR("\":"));
        sprintf(tempBuf, "%.2f", f1);
        tempSpan = az_span_create_from_str(tempBuf);
        payload = az_span_copy(payload, tempSpan);
        found = true;
    }

    if(b2) {
        payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"p\":"));
        sprintf(tempBuf, "%.2f", f2);
        tempSpan2 = az_span_create_from_str(tempBuf);
        payload = az_span_copy(payload, tempSpan2);
        found = true;
    }
    if(b3) {
        payload = az_span_copy(payload, AZ_SPAN_FROM_STR(",\"h\":"));
        sprintf(tempBuf, "%.2f", f3);
        tempSpan2 = az_span_create_from_str(tempBuf);
        payload = az_span_copy(payload, tempSpan2);
        found = true;
    }
  }

  payload = az_span_copy(payload, AZ_SPAN_FROM_STR("}"));
  payload = az_span_copy_u8(payload, '\0');
  
  *out_payload = az_span_slice(original_payload, 0, az_span_size(original_payload) - az_span_size(payload) - 1);
  return found;
}

char *Telemetry::getDataBuf() {
    return this->dataBuf;
}


int Telemetry::getNumStored() {
    return *(this->numStored);
}

int Telemetry::getRemainingSize() {
    return this->bufSize-(*this->bytesStored);
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

void Telemetry::addSensor(BaseSensor *s) {
    this->sensors[this->numSensors++] = s;
}
