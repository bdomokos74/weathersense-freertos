// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _SLLOGGER_H
#define _SLLOGGER_H

#include <az_core.h>
#include "esp_log.h"

class Logger
{
private:
  void writeTime(char *buf);
public:
  Logger();
  
  void printBuf(const char *tag, const char *header, const char *data, int len);
};

extern Logger logger;

#endif // _SLLOGGER_H
