// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "Logger.h"

Logger::Logger() {
}

void Logger::printBuf(const char *tag, const char *header, const char *data, int len) {
    char buf[1024];
    az_span bufspan = AZ_SPAN_FROM_BUFFER(buf);
    az_span rem = az_span_copy(bufspan, az_span_create_from_str((char*)header));
    rem = az_span_copy(rem, az_span_create((uint8_t*)data, len));
    az_span_copy_u8(rem, 0);    
    ESP_LOGI(tag, " %s", buf);
}

Logger logger;
