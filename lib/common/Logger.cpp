// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "Logger.h"
#include <time.h>
#include <az_span.h>
#include <stdlib.h>
#include "esp_log.h"

#define TAG "log"

#define UNIX_EPOCH_START_YEAR 1900

static void writeTime(char *buf)
{
  struct tm* ptm;
  time_t now = time(NULL);
  ptm = gmtime(&now);

  sprintf(buf, "%d/%d/%d %02d:$02d:%02d %s",  ptm->tm_year + UNIX_EPOCH_START_YEAR, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

Logger::Logger() { Serial.begin(SERIAL_LOGGER_BAUD_RATE); }

void Logger::info(char* message)
{
  char timebuf[32];
  writeTime(timebuf);
  ESP_LOGI(TAG, "%s - %s\n", timebuf, message);

}

void Logger::error(char* message)
{
  char timebuf[32];
  writeTime(timebuf);
  ESP_LOGE(TAG, "%s - %s\n", timebuf, message);
}

void Logger::printBuf(char *header, char *data, int len) {
    az_span span;
    char buf[1024];
    az_span bufspan = AZ_SPAN_FROM_BUFFER(buf);
    az_span rem = az_span_copy(bufspan, az_span_create_from_str(header));
    rem = az_span_copy(rem, az_span_create((uint8_t*)data, len));
    az_span_copy_u8(rem, 0);
    
    char timebuf[32];
    writeTime(timebuf);
    writeTime(timebuf);
    ESP_LOGI(TAG, " %s -%s\n", timebuf, buf);
}

void Logger::println(char *str) {
  char timebuf[32];
  writeTime(timebuf);
  
  ESP_LOGI(TAG, "%s - %s\n", timebuf, str);
}

void Logger::println(char *str1, char *str2) {
  char timebuf[32];
  writeTime(timebuf);
  
  ESP_LOGI(TAG, "%s - %s %s\n", timebuf, str1, str2);
}

void Logger::println(char *str1, int i) {
  char timebuf[32];
  writeTime(timebuf);
  
  ESP_LOGI(TAG, "%s - %s %d\n", timebuf, str1, i);
}


Logger Logger;
