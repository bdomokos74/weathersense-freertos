// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _SLLOGGER_H
#define _SLLOGGER_H

#include <az_core.h>
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

void logBuf(const char *tag, const char *header, const char *data, int len);
void logSpan(const char *tag, const char *header, az_span span);
unsigned logHWMIfHigher1(unsigned prevMax, char *fname, int line);
void logHWM1(char *fname, int line);

#define logHWM() \
    logHWM1(__FILE__, __LINE__);

#define logHWMIfHigher(prevMax) \
    logHWMIfHigher1(prevMax, __FILE__, __LINE__);


#ifdef __cplusplus
}
#endif

#endif