// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef SERIALLOGGER_H
#define SERIALLOGGER_H

#ifndef SERIAL_LOGGER_BAUD_RATE
#define SERIAL_LOGGER_BAUD_RATE 115200
#endif

class Logger
{
public:
  Logger();
  void info(String message);
  void error(String message);
  void printBuf(char *header, char *data, int len);
  void println(char *str1, char *str2);
  void println(char *str1, int i);
  void println(char *str);
};

extern Logger Logger;

#endif // SERIALLOGGER_H
