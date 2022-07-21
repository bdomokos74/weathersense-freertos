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
  (void)az_span_u32toa(payload, (*(this->telemetryId))++, &payload);

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
