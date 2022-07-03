// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "logger.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static char lbuf[512];
void logBuf(const char *tag, const char *header, const char *data, int len) {

    az_span bufspan = AZ_SPAN_FROM_BUFFER(lbuf);
    az_span rem = az_span_copy(bufspan, az_span_create_from_str((char*)header));
    rem = az_span_copy(rem, az_span_create((uint8_t*)data, len));
    az_span_copy_u8(rem, 0);    
    ESP_LOGI(tag, "%s", lbuf);
}

void logSpan(const char *tag, const char *header, az_span span) {
    logBuf(tag, header, (char*)az_span_ptr(span), az_span_size(span));
}

unsigned logHWMIfHigher1(unsigned prevMax, char *fname, int line) {
    TaskStatus_t xTaskDetails;
    vTaskGetInfo(NULL,
                  &xTaskDetails,
                  pdTRUE,
                  eInvalid );

    if(xTaskDetails.usStackHighWaterMark > prevMax) {
        ESP_LOGI((const char*)xTaskDetails.pcTaskName, "HWM UP=%d - at %s:%d", xTaskDetails.usStackHighWaterMark, fname, line);
        return xTaskDetails.usStackHighWaterMark;
    } else {
        return prevMax;
    }
}


void logHWM1(char *fname, int line) {
    TaskStatus_t xTaskDetails;
    vTaskGetInfo(NULL,
                  &xTaskDetails,
                  pdTRUE,
                  eInvalid );
    ESP_LOGI((const char*)xTaskDetails.pcTaskName, "HWM=%d - at %s:%d", xTaskDetails.usStackHighWaterMark, fname, line);
}